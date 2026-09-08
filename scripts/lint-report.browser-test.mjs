// Optional UI regression runner. Requires an existing Playwright installation;
// this script never installs packages or downloads browsers.
// NODE_PATH may point to an existing node_modules directory.
import assert from 'node:assert/strict';
import { readFile, writeFile, mkdir } from 'node:fs/promises';
import { resolve, dirname } from 'node:path';
import { createRequire } from 'node:module';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { compactReport, renderReport } from './preview-lint-report.mjs';

const require = createRequire(import.meta.url);
const { chromium } = require('playwright');
const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const output = resolve(root, '.build/report-browser-tests');
await mkdir(output, { recursive: true });
const template = await readFile(resolve(root, 'Plugins/PositiveLinter/Resources/LintReportTemplate.html'), 'utf8');
const hostileText = '</script><img src="https://should-never-load.invalid/x" onerror="window.injected=true"><script>window.injected=true</script> 日本語\n"quoted", value';
const legacy = { Violators: Array.from({ length: 120 }, (_, index) => {
  const name = `Asset${String(index).padStart(3, '0')}`;
  const className = index % 2 ? 'Texture2D' : 'Blueprint';
  return {
    ViolatorAssetName: name,
    ViolatorAssetPath: `/Game/Tests/${name}.${name}`,
    ViolatorFullName: `/Script/Engine.${className} /Game/Tests/${name}.${name}`,
    Violations: [{
      RuleGroup: index % 2 ? 'Textures' : 'Naming',
      RuleTitle: `Rule ${index % 3}`,
      RuleDesc: index === 0 ? hostileText : 'A useful description.',
      RuleURL: index === 0 ? 'javascript:window.injected=true' : 'https://example.invalid/rules',
      RuleSeverity: index % 3,
      RuleRecommendedAction: index === 0 ? hostileText : `Fix ${name}.`,
    }],
  };
}) };
const reportPath = resolve(output, 'fixture.html');
await writeFile(reportPath, renderReport(template, compactReport(legacy, { project: 'Report regression', generatedAt: '2026-09-07T12:00:00Z' })), 'utf8');

const browser = await chromium.launch({
  headless: true,
  ...(process.env.BROWSER_EXECUTABLE_PATH ? { executablePath: process.env.BROWSER_EXECUTABLE_PATH } : { channel: 'msedge' }),
});
const page = await browser.newPage({ viewport: { width: 1440, height: 1000 } });
const errors = [];
const requests = [];
page.on('pageerror', error => errors.push(error.message));
page.on('request', request => { if (/^https?:/.test(request.url())) requests.push(request.url()); });
await page.route(/^https?:/, route => route.abort());

const rows = page.locator('#results-body tr[data-row]');
const numericText = async selector => Number((await page.locator(selector).innerText()).replace(/[^\d]/g, ''));
async function assertRows(count) {
  await page.waitForFunction(expected => document.querySelectorAll('#results-body tr[data-row]').length === expected, count);
  assert.equal(await rows.count(), count);
}
async function clearFilters() {
  await page.locator('#clear-filters').click();
  await page.locator('[data-severity="all"]').first().click();
  await page.locator('#search').fill('');
}

try {
  await page.goto(pathToFileURL(reportPath).href);
  await page.waitForSelector('body[data-ready="true"]');
  assert.equal(await numericText('#total-findings'), 120);
  assert.equal(await numericText('#total-assets'), 120);
  assert.equal(await numericText('#total-errors'), 40);
  assert.equal(await numericText('#total-warnings'), 40);
  await page.locator('#page-size').selectOption('25');
  await assertRows(25);
  assert.equal(await page.locator('#detail-panel').isVisible(), false);

  for (const severity of ['0', '1', '2']) {
    await page.locator(`[data-severity="${severity}"]`).first().click();
    assert.match(await page.locator('#result-count').innerText(), /40/);
    await assertRows(25);
  }
  await clearFilters();

  const classValue = await page.locator('#class-filter option').evaluateAll(options => options.find(option => option.textContent.includes('Texture2D'))?.value);
  assert.ok(classValue, 'Texture2D class filter exists');
  await page.locator('#class-filter').selectOption(classValue);
  assert.match(await page.locator('#result-count').innerText(), /60/);
  await page.locator('[data-severity="1"]').first().click();
  await assertRows(20);
  await clearFilters();

  const groupValue = await page.locator('#group-filter option').evaluateAll(options => options.find(option => option.textContent.includes('Naming'))?.value);
  assert.ok(groupValue, 'Naming rule group filter exists');
  await page.locator('#group-filter').selectOption(groupValue);
  assert.match(await page.locator('#result-count').innerText(), /60/);
  await clearFilters();

  await page.locator('#search').fill('asset017');
  await assertRows(1);
  assert.match(await rows.first().innerText(), /Asset017/);
  await page.locator('#search').fill('does-not-exist-4829');
  await assertRows(0);
  assert.equal(await page.locator('#empty-state').isVisible(), true);
  await clearFilters();

  await page.locator('[data-sort="asset"]').click();
  assert.match(await rows.first().innerText(), /Asset000/);
  await page.locator('[data-sort="asset"]').click();
  assert.match(await rows.first().innerText(), /Asset119/);
  await page.locator('[data-sort="asset"]').click();
  await page.locator('#next-page').click();
  assert.match(await rows.first().innerText(), /Asset025/);
  await page.locator('#prev-page').click();
  assert.match(await rows.first().innerText(), /Asset000/);
  await page.locator('#page-size').selectOption('100');
  await assertRows(100);
  await page.locator('#next-page').click();
  await assertRows(20);
  assert.equal(await page.locator('#next-page').isDisabled(), true);
  await page.locator('#prev-page').click();

  await rows.first().locator('button').focus();
  await page.keyboard.press('Enter');
  assert.equal(await page.locator('#detail-panel').isVisible(), true);
  assert.ok((await page.locator('#detail-panel').innerText()).includes(hostileText));
  assert.equal(await page.locator('#detail-panel img, #detail-panel script').count(), 0);
  assert.equal(await page.locator('#detail-panel a[href^="javascript:"]').count(), 0);
  assert.equal(await page.evaluate(() => window.injected), undefined);
  await page.keyboard.press('Escape');
  assert.equal(await page.locator('#detail-panel').isVisible(), false);

  await page.locator('#search').fill('Asset017');
  await assertRows(1);
  const downloadPromise = page.waitForEvent('download');
  await page.locator('#export-csv').click();
  const download = await downloadPromise;
  const csvPath = resolve(output, 'filtered.csv');
  await download.saveAs(csvPath);
  const csv = await readFile(csvPath, 'utf8');
  assert.ok(csv.includes('Asset017'));
  assert.ok(!csv.includes('Asset018'), 'CSV exports the filtered result set');

  await clearFilters();
  await page.setViewportSize({ width: 390, height: 844 });
  assert.equal(await page.locator('#search').isVisible(), true);
  await page.locator('#search').fill('Asset003');
  await assertRows(1);
  await rows.first().click();
  assert.equal(await page.locator('#detail-panel').isVisible(), true);
  await page.locator('#detail-close').click();

  const emptyPath = resolve(output, 'empty.html');
  await writeFile(emptyPath, renderReport(template, compactReport({ Violators: [] }, { project: 'Empty report' })), 'utf8');
  await page.goto(pathToFileURL(emptyPath).href);
  await page.waitForSelector('body[data-ready="true"]');
  assert.equal(await numericText('#total-findings'), 0);
  await assertRows(0);
  assert.equal(await page.locator('#empty-state').isVisible(), true);
  assert.deepEqual(errors, [], 'No browser JavaScript errors');
  assert.deepEqual(requests, [], 'Standalone reports make no network requests');
  console.log('PASS: counts, severity/class/group/search filters, ascending/descending sorting, pagination, keyboard details, safe diagnostics/links, filtered CSV, mobile access, empty reports, and offline rendering.');
} finally {
  await browser.close();
}
