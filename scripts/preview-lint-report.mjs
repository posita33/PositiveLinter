import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { basename, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const repositoryRoot = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const defaultTemplate = resolve(repositoryRoot, 'Plugins/PositiveLinter/Resources/LintReportTemplate.html');

export function decodeReportFile(bytes) {
  if (bytes[0] === 0xff && bytes[1] === 0xfe) return new TextDecoder('utf-16le', { fatal: true }).decode(bytes);
  if (bytes[0] === 0xfe && bytes[1] === 0xff) return new TextDecoder('utf-16be', { fatal: true }).decode(bytes);
  return new TextDecoder('utf-8', { fatal: true }).decode(bytes);
}

// Extract only JSON. Never execute JavaScript found in an imported HTML report.
export function parseReport(source) {
  const trimmed = source.trim().replace(/^\uFEFF/, '');
  if (trimmed.startsWith('{')) return JSON.parse(trimmed);
  const embedded = /<script\b(?=[^>]*\bid\s*=\s*["']report-data["'])[^>]*>([\s\S]*?)<\/script\s*>/i.exec(source);
  if (embedded) return JSON.parse(embedded[1]);
  const assignment = /\b(?:var|let|const)\s+report\s*=\s*(?=\{)/.exec(source);
  if (!assignment) throw new Error('No JSON report data found in this file.');
  const start = assignment.index + assignment[0].length;
  let depth = 0;
  let inString = false;
  let escaped = false;
  for (let index = start; index < source.length; index += 1) {
    const character = source[index];
    if (inString) {
      if (escaped) escaped = false;
      else if (character === '\\') escaped = true;
      else if (character === '"') inString = false;
    } else if (character === '"') inString = true;
    else if (character === '{') depth += 1;
    else if (character === '}' && --depth === 0) return JSON.parse(source.slice(start, index + 1));
  }
  throw new Error('Report JSON is incomplete.');
}

function string(value, field) {
  if (typeof value !== 'string') throw new Error(`Expected a string for ${field}.`);
  return value;
}

function validateCompactReport(report) {
  if (!Array.isArray(report.assets) || !Array.isArray(report.rules) || !Array.isArray(report.strings)) {
    throw new Error('The version 2 report is missing its assets, rules, or strings.');
  }
  report.strings.forEach((value, index) => string(value, `strings[${index}]`));
  const reference = (value, count, field) => {
    if (!Number.isInteger(value) || value < 0 || value >= count) throw new Error(`Invalid reference for ${field}.`);
  };
  const tuple = (value, size, field) => {
    if (!Array.isArray(value) || value.length !== size) throw new Error(`Expected ${size} fields for ${field}.`);
  };
  report.rules.forEach((rule, index) => {
    tuple(rule, 5, `rules[${index}]`);
    for (let field = 0; field < 4; field += 1) reference(rule[field], report.strings.length, `rules[${index}][${field}]`);
    if (!Number.isInteger(rule[4]) || rule[4] < 0 || rule[4] > 2) throw new Error(`Unsupported severity in rules[${index}]: ${rule[4]}.`);
  });
  report.assets.forEach((asset, index) => {
    tuple(asset, 4, `assets[${index}]`);
    for (let field = 0; field < 3; field += 1) reference(asset[field], report.strings.length, `assets[${index}][${field}]`);
    if (!Array.isArray(asset[3])) throw new Error(`Expected a findings array for assets[${index}][3].`);
    asset[3].forEach((finding, findingIndex) => {
      const location = `assets[${index}][3][${findingIndex}]`;
      tuple(finding, 2, location);
      reference(finding[0], report.rules.length, `${location}[0]`);
      reference(finding[1], report.strings.length, `${location}[1]`);
    });
  });
  for (const field of ['project', 'generatedAt', 'ruleSet']) {
    if (report[field] !== undefined) string(report[field], field);
  }
  if (report.paths !== undefined && (!Array.isArray(report.paths) || report.paths.some(value => typeof value !== 'string'))) {
    throw new Error('Expected a string array for paths.');
  }
}

export function compactReport(report, metadata = {}) {
  if (!report || typeof report !== 'object' || Array.isArray(report)) throw new Error('Expected a report object.');
  if (report.version === 2) {
    validateCompactReport(report);
    return report;
  }
  if (!Array.isArray(report.Violators)) throw new Error('Unsupported report format: expected Violators or version 2.');
  const strings = [];
  const stringIds = new Map();
  const rules = [];
  const ruleIds = new Map();
  const intern = value => {
    if (!stringIds.has(value)) {
      stringIds.set(value, strings.length);
      strings.push(value);
    }
    return stringIds.get(value);
  };
  const assets = report.Violators.map((asset, assetIndex) => {
    const name = string(asset.ViolatorAssetName, `asset ${assetIndex} name`);
    const path = string(asset.ViolatorAssetPath, `asset ${assetIndex} path`);
    const fullName = string(asset.ViolatorFullName, `asset ${assetIndex} full name`);
    const separator = fullName.indexOf(' ');
    if (separator < 1 || fullName.slice(separator + 1) !== path) {
      throw new Error(`Cannot preserve the class/path of asset ${assetIndex}: invalid full name.`);
    }
    if (!Array.isArray(asset.Violations)) throw new Error(`Asset ${assetIndex} has no violations array.`);
    const findings = asset.Violations.map((violation, findingIndex) => {
      const location = `asset ${assetIndex}, finding ${findingIndex}`;
      const severity = violation.RuleSeverity;
      if (!Number.isInteger(severity) || severity < 0 || severity > 2) {
        throw new Error(`Unsupported severity in ${location}: ${severity}.`);
      }
      const rule = [
        intern(string(violation.RuleGroup, `${location} group`)),
        intern(string(violation.RuleTitle, `${location} title`)),
        intern(string(violation.RuleDesc, `${location} description`)),
        intern(string(violation.RuleURL, `${location} URL`)),
        severity,
      ];
      const key = JSON.stringify(rule);
      if (!ruleIds.has(key)) {
        ruleIds.set(key, rules.length);
        rules.push(rule);
      }
      return [ruleIds.get(key), intern(string(violation.RuleRecommendedAction, `${location} action`))];
    });
    return [intern(name), intern(path), intern(fullName.slice(0, separator)), findings];
  });
  return {
    version: 2,
    project: metadata.project || 'Unreal Project',
    generatedAt: metadata.generatedAt || '',
    ruleSet: metadata.ruleSet || 'Legacy report',
    paths: metadata.paths || [],
    strings,
    rules,
    assets,
  };
}

function decodeHtmlText(value) {
  const entities = { amp: '&', lt: '<', gt: '>', quot: '"', apos: "'" };
  return value.replace(/&(#x[0-9a-f]+|#\d+|amp|lt|gt|quot|apos);/gi, (match, entity) => {
    if (entity[0] !== '#') return entities[entity.toLowerCase()] || match;
    const code = entity[1].toLowerCase() === 'x' ? parseInt(entity.slice(2), 16) : parseInt(entity.slice(1), 10);
    return code <= 0x10ffff ? String.fromCodePoint(code) : match;
  });
}

export function inferMetadata(source, inputPath) {
  const title = /<title\b[^>]*>([\s\S]*?)<\/title\s*>/i.exec(source)?.[1] || '';
  const timestamp = /lint-report-(\d{4})\.(\d{2})\.(\d{2})-(\d{2})\.(\d{2})\.(\d{2})/.exec(basename(inputPath));
  return {
    project: decodeHtmlText(title).replace(/\s+Lint Report\s*$/i, '').trim() || 'Unreal Project',
    generatedAt: timestamp ? `${timestamp[1]}-${timestamp[2]}-${timestamp[3]} ${timestamp[4]}:${timestamp[5]}:${timestamp[6]}` : '',
  };
}

export function reportJson(report) {
  // A literal closing script tag must never escape the application/json element.
  return JSON.stringify(report).replace(/[<>&\u2028\u2029]/g, character => `\\u${character.charCodeAt(0).toString(16).padStart(4, '0')}`);
}

export function renderReport(template, report) {
  if (!template.includes('{% LINT_REPORT %}') || !template.includes('{% TITLE %}')) {
    throw new Error('Template must contain {% TITLE %} and {% LINT_REPORT %}.');
  }
  const title = String(report.project || 'Unreal Project');
  const escapedTitle = title.replace(/[&<>"']/g, character => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' })[character]);
  // Callback replacements preserve diagnostic text containing "$&" or "$`".
  const json = reportJson(report);
  return template.replace(/\{% (TITLE|LINT_REPORT) %\}/g, (_match, token) => token === 'TITLE' ? escapedTitle : json);
}

export function reportSummary(report) {
  const severities = [0, 0, 0];
  for (const asset of report.assets) {
    for (const finding of asset[3]) severities[report.rules[finding[0]][4]] += 1;
  }
  return { assets: report.assets.length, findings: severities.reduce((sum, count) => sum + count, 0), errors: severities[0], warnings: severities[1], info: severities[2], uniqueRules: report.rules.length, uniqueStrings: report.strings.length };
}

async function main() {
  const args = process.argv.slice(2);
  if (args.includes('--help') || args.includes('-h')) {
    console.log('Usage: node scripts/preview-lint-report.mjs <input.html|input.json> [output.html] [--template template.html]\nDefault output: .build/report-preview/index.html\nReads report data as JSON only. The original report is never overwritten.');
    return;
  }
  let templatePath = defaultTemplate;
  const templateIndex = args.indexOf('--template');
  if (templateIndex >= 0) {
    if (!args[templateIndex + 1]) throw new Error('--template needs a file path.');
    templatePath = resolve(args[templateIndex + 1]);
    args.splice(templateIndex, 2);
  }
  if (args.length < 1 || args.length > 2 || args.some(argument => argument.startsWith('--'))) {
    throw new Error('Usage: node scripts/preview-lint-report.mjs <input.html|input.json> [output.html] [--template template.html]');
  }
  const inputPath = resolve(args[0]);
  const outputPath = resolve(args[1] || resolve(repositoryRoot, '.build/report-preview/index.html'));
  if (inputPath.toLowerCase() === outputPath.toLowerCase() || templatePath.toLowerCase() === outputPath.toLowerCase()) {
    throw new Error('Output must differ from the input report and template.');
  }
  const [sourceBytes, template] = await Promise.all([readFile(inputPath), readFile(templatePath, 'utf8')]);
  const source = decodeReportFile(sourceBytes);
  const report = compactReport(parseReport(source), inferMetadata(source, inputPath));
  const summary = reportSummary(report);
  const html = renderReport(template, report);
  await mkdir(dirname(outputPath), { recursive: true });
  await writeFile(outputPath, html, 'utf8');
  const before = sourceBytes.length;
  const after = Buffer.byteLength(html);
  console.log(JSON.stringify({ output: outputPath, ...summary, inputBytes: before, outputBytes: after, reductionPercent: Number(((1 - after / before) * 100).toFixed(2)) }, null, 2));
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  main().catch(error => {
    console.error(`Report preview failed: ${error.message}`);
    process.exitCode = 1;
  });
}
