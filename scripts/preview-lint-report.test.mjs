import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdir, mkdtemp, readFile, rm, writeFile } from 'node:fs/promises';
import { spawnSync } from 'node:child_process';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { compactReport, decodeReportFile, inferMetadata, parseReport, renderReport, reportJson, reportSummary } from './preview-lint-report.mjs';

function makeLegacy() {
  const violation = {
    RuleGroup: 'Naming', RuleTitle: 'Name correctly', RuleDesc: 'A shared description',
    RuleURL: 'https://example.invalid/rules', RuleSeverity: 0, RuleRecommendedAction: 'Rename the asset.',
  };
  return { Violators: ['A', 'B'].map(name => ({
    ViolatorAssetName: name,
    ViolatorAssetPath: `/Game/${name}.${name}`,
    ViolatorFullName: `/Script/Engine.Blueprint /Game/${name}.${name}`,
    Violations: [structuredClone(violation), { ...violation, RuleSeverity: 1 }],
  })) };
}

function expandReport(report) {
  const strings = report.strings;
  return { Violators: report.assets.map(asset => ({
    ViolatorAssetName: strings[asset[0]],
    ViolatorAssetPath: strings[asset[1]],
    ViolatorFullName: `${strings[asset[2]]} ${strings[asset[1]]}`,
    Violations: asset[3].map(([ruleId, actionId]) => {
      const rule = report.rules[ruleId];
      return { RuleGroup: strings[rule[0]], RuleTitle: strings[rule[1]], RuleDesc: strings[rule[2]], RuleURL: strings[rule[3]], RuleSeverity: rule[4], RuleRecommendedAction: strings[actionId] };
    }),
  })) };
}

test('legacy conversion is lossless and shares repeated rules and strings', () => {
  const legacy = makeLegacy();
  const compact = compactReport(legacy);
  assert.deepEqual(expandReport(compact), legacy);
  assert.equal(compact.rules.length, 2);
  assert.equal(compact.strings.filter(value => value === 'A shared description').length, 1);
  assert.deepEqual(reportSummary(compact), { assets: 2, findings: 4, errors: 2, warnings: 2, info: 0, uniqueRules: 2, uniqueStrings: 10 });
});

test('legacy JSON scanner handles braces, escaped quotes, and backslashes in strings', () => {
  const data = makeLegacy();
  data.Violators[0].Violations[0].RuleRecommendedAction = '日本語: {value} "quoted" \\ path }';
  const html = `<script>var report = ${JSON.stringify(data)}; throw new Error('never executed');</script>`;
  assert.deepEqual(parseReport(html), data);
});

test('JavaScript expressions and incomplete input are rejected without evaluation', () => {
  globalThis.reportParserExecuted = false;
  assert.throws(() => parseReport('<script>var report = {"Violators":(globalThis.reportParserExecuted = true,[])};</script>'));
  assert.equal(globalThis.reportParserExecuted, false);
  assert.throws(() => parseReport('<script>var report = {"Violators": ['), /incomplete/);
  assert.throws(() => parseReport('<html>no report</html>'), /No JSON report/);
  delete globalThis.reportParserExecuted;
});

test('UTF-16 legacy files and UTF-8 reports decode without corrupting Japanese', () => {
  const source = '{"text":"日本語 🎮"}';
  const utf16 = Buffer.concat([Buffer.from([0xff, 0xfe]), Buffer.from(source, 'utf16le')]);
  const utf16be = Buffer.from(utf16).swap16();
  assert.equal(decodeReportFile(utf16), source);
  assert.equal(decodeReportFile(utf16be), source);
  assert.equal(decodeReportFile(Buffer.from(source)), source);
  assert.throws(() => decodeReportFile(Buffer.from([0xc3, 0x28])));
});

test('JSON cannot break out of its script element and replacement tokens remain literal', () => {
  const report = compactReport(makeLegacy(), { project: '<img onerror="x"> & 日本語 {% LINT_REPORT %}' });
  const hostile = '</script><script>globalThis.injected=true</script> $& $` $\' \u2028\u2029';
  report.strings.push(hostile);
  const template = '<title>{% TITLE %}</title><script type="application/json" id="report-data">{% LINT_REPORT %}</script>';
  const html = renderReport(template, report);
  assert.equal((html.match(/<script/g) || []).length, 1);
  assert.equal((html.match(/<\/script>/g) || []).length, 1);
  assert.ok(html.includes('&lt;img onerror=&quot;x&quot;&gt; &amp; 日本語'));
  assert.ok(html.includes('日本語 {% LINT_REPORT %}</title>'));
  assert.deepEqual(parseReport(html), report);
  assert.equal(JSON.parse(reportJson(report)).strings.at(-1), hostile);
});

test('missing metadata stays unknown and the original local timestamp is not relabeled UTC', () => {
  assert.deepEqual(inferMetadata('<title>A &amp; B Lint Report</title>', 'lint-report-2026.09.07-21.01.06.html'), { project: 'A & B', generatedAt: '2026-09-07 21:01:06' });
  assert.deepEqual(inferMetadata('', 'report.json'), { project: 'Unreal Project', generatedAt: '' });
  const compact = compactReport({ Violators: [] });
  assert.deepEqual(compact.paths, []);
  assert.equal(compact.generatedAt, '');
  assert.equal(compact.ruleSet, 'Legacy report');
});

test('ambiguous full names, missing fields, and unknown severities fail cleanly', () => {
  const legacy = makeLegacy();
  legacy.Violators[0].ViolatorFullName = 'different data';
  assert.throws(() => compactReport(legacy), /Cannot preserve/);
  legacy.Violators[0].ViolatorFullName = '/Script/Engine.Blueprint /Game/A.A';
  delete legacy.Violators[0].Violations[0].RuleTitle;
  assert.throws(() => compactReport(legacy), /Expected a string/);
  legacy.Violators[0].Violations[0].RuleTitle = 'Title';
  legacy.Violators[0].Violations[0].RuleSeverity = 99;
  assert.throws(() => compactReport(legacy), /Unsupported severity/);
});

test('valid version 2 data is retained and malformed templates are rejected', () => {
  const compact = compactReport(makeLegacy());
  assert.equal(compactReport(compact), compact);
  assert.throws(() => compactReport({ version: 2 }), /missing/);
  assert.throws(() => compactReport({ version: 3 }), /Unsupported report format/);
  assert.throws(() => renderReport('<html></html>', compact), /Template must contain/);
});

test('version 2 validates every reference, tuple, severity, and metadata type', () => {
  const mutations = [
    [report => { report.rules[0][0] = report.strings.length; }, /Invalid reference/],
    [report => { report.assets[0][0] = -1; }, /Invalid reference/],
    [report => { report.assets[0][3][0][1] = 0.5; }, /Invalid reference/],
    [report => { report.assets[0][3][0][0] = '0'; }, /Invalid reference/],
    [report => { report.rules[0][4] = 3; }, /Unsupported severity/],
    [report => { report.rules[0][4] = 0.5; }, /Unsupported severity/],
    [report => { report.rules[0].push('extra'); }, /Expected 5 fields/],
    [report => { report.assets[0][3][0].pop(); }, /Expected 2 fields/],
    [report => { report.assets[0][3] = null; }, /findings array/],
    [report => { report.strings[0] = null; }, /Expected a string/],
    [report => { report.project = {}; }, /Expected a string/],
    [report => { report.paths = [42]; }, /string array/],
  ];
  for (const [mutate, expected] of mutations) {
    const report = compactReport(makeLegacy());
    mutate(report);
    assert.throws(() => compactReport(report), expected);
  }
  assert.throws(() => compactReport(null), /report object/);
  assert.throws(() => compactReport([]), /report object/);
});

test('invalid version 2 input fails before overwriting an existing output file', async context => {
  const repositoryRoot = resolve(dirname(fileURLToPath(import.meta.url)), '..');
  const buildDirectory = resolve(repositoryRoot, '.build');
  await mkdir(buildDirectory, { recursive: true });
  const fixtureDirectory = await mkdtemp(resolve(buildDirectory, 'report-converter-test-'));
  context.after(() => rm(fixtureDirectory, { recursive: true, force: true }));
  const inputPath = resolve(fixtureDirectory, 'input.json');
  const outputPath = resolve(fixtureDirectory, 'existing.html');
  const templatePath = resolve(fixtureDirectory, 'template.html');
  await writeFile(outputPath, 'previous report must be preserved', 'utf8');
  await writeFile(templatePath, '<title>{% TITLE %}</title><script type="application/json" id="report-data">{% LINT_REPORT %}</script>', 'utf8');
  const mutations = [
    report => { report.assets[0][3][0][0] = report.rules.length; },
    report => { report.assets[0][3][0][1] = report.strings.length; },
    report => { report.rules[0][4] = 99; },
  ];
  for (const mutate of mutations) {
    const report = compactReport(makeLegacy());
    mutate(report);
    await writeFile(inputPath, JSON.stringify(report), 'utf8');
    const result = spawnSync(process.execPath, [resolve(repositoryRoot, 'scripts/preview-lint-report.mjs'), inputPath, outputPath, '--template', templatePath], { encoding: 'utf8' });
    assert.equal(result.status, 1);
    assert.match(result.stderr, /Report preview failed: (Invalid reference|Unsupported severity)/);
    assert.equal(await readFile(outputPath, 'utf8'), 'previous report must be preserved');
  }
});
