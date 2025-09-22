// ***************************************************************************
// Copyright (c) 2021 SAP SE or an SAP affiliate company. All rights reserved.
// ***************************************************************************
// This sample code is provided AS IS, without warranty or liability of any kind.
//
// You may use, reproduce, modify and distribute this sample code without limitation,
// on the condition that you retain the foregoing copyright notice and disclaimer
// as to the original code.
// ***************************************************************************

'use strict'

const assert = require('assert')
const crypto = require('crypto') // For uniqueidentifier
require('dotenv').config()
// Load the compiled addon directly
const sqlanywhere = require('../promise')

// --- Configuration ---
const connParams = {
  ServerName: process.env.DB_SERVER || 'database',
  UserID: process.env.DB_USER || 'username',
  Password: process.env.DB_PASSWORD || 'password'
}

const testTableName = 'NAPI_DRIVER_TEST'
const testProcName = 'GetProductInfo'
const updateProcName = 'UpdateProductName'
const multiResultProcName = 'GetMultipleResults'

// --- Individual Test Functions ---

async function testCreateTable(db) {
  console.time('Create Table Duration')
  console.log(`\n[3/10] Creating test table '${testTableName}'...`)
  await db.exec(`
      CREATE TABLE ${testTableName} (
        id_pk INT PRIMARY KEY,
        c_bigint BIGINT DEFAULT 0,
        c_binary BINARY(16) DEFAULT NULL,
        c_bit BIT DEFAULT 0,
        c_char CHAR(10) DEFAULT '',
        c_date DATE DEFAULT '1900-01-01',
        c_decimal DECIMAL(10, 5) DEFAULT 0,
        c_double DOUBLE DEFAULT 0,
        c_float FLOAT DEFAULT 0,
        c_integer INTEGER DEFAULT 0,
        c_long_binary LONG BINARY DEFAULT NULL,
        c_long_nvarchar LONG NVARCHAR DEFAULT '',
        c_long_varchar LONG VARCHAR DEFAULT '',
        c_numeric NUMERIC(12, 6) DEFAULT 0,
        c_smallint SMALLINT DEFAULT 0,
        c_st_geometry ST_GEOMETRY DEFAULT NULL,
        c_time TIME DEFAULT '00:00:00',
        c_timestamp TIMESTAMP DEFAULT NOW(),
        c_timestamp_tz TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
        c_tinyint TINYINT DEFAULT 0,
        c_uniqueidentifier UNIQUEIDENTIFIER DEFAULT NEWID(),
        c_unsigned_bigint UNSIGNED BIGINT DEFAULT 0,
        c_unsigned_int UNSIGNED INT DEFAULT 0,
        c_unsigned_smallint UNSIGNED SMALLINT DEFAULT 0,
        c_varbinary VARBINARY(20) DEFAULT NULL,
        c_varchar VARCHAR(100) DEFAULT '',
        c_xml XML DEFAULT NULL
      )`)
  await db.commit()
  console.log('    Table created.')
  console.timeEnd('Create Table Duration')
}

async function testInsertAndCommit(db) {
  console.time('Insert and Commit Duration')
  console.log('\n[4/10] Testing INSERT and COMMIT...')
  const uuid = crypto.randomUUID()
  const wktGeometry = 'POINT (10 20)'
  const xmlData = '<root><item id="1">test</item></root>'

  const insertSQL = `
      INSERT INTO ${testTableName} VALUES (
        ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
        ?, ?, ?, ?, ?, ?, ?
      )`
  const insertParams = [1, 9223372036854775807, Buffer.from('0123456789ABCDEF', 'hex'), 1, 'fixedchar', '2025-09-17', 12345.12345, 1.23456789e10, 1.23e4, 2147483647, Buffer.from('This is a long binary value'), '這是長的NVARCHAR值', 'This is a long varchar value', 987654.123456, 32767, wktGeometry, '21:22:23', '2025-09-17 21:22:23.123', '2025-09-17 21:22:23.456-05:00', 127, uuid, 18446744073709551615n, 4294967295, 65535, Buffer.from('variable binary data'), 'Variable character data', xmlData]

  let affectedRows = await db.exec(
    insertSQL,
    insertParams.map((p) => (typeof p === 'bigint' ? p.toString() : p))
  )
  assert.strictEqual(affectedRows, 1, 'INSERT should affect 1 row.')
  await db.commit()
  console.log('    Data inserted and committed.')

  let result = await db.exec(`SELECT * FROM ${testTableName} WHERE id_pk = 1`)
  assert.strictEqual(result[0].c_uniqueidentifier.toUpperCase(), uuid.toUpperCase(), 'UNIQUEIDENTIFIER mismatch.')
  assert.strictEqual(result[0].c_xml, xmlData, 'XML mismatch.')
  assert.strictEqual(result[0].c_st_geometry.toUpperCase(), wktGeometry.toUpperCase(), 'ST_GEOMETRY mismatch.')
  console.log('    Data verified after commit.')
  console.timeEnd('Insert and Commit Duration')
}

async function testRollback(db) {
  console.time('Rollback Duration')
  console.log(`\n[5/10] Testing ROLLBACK...`)
  await db.exec(`INSERT INTO ${testTableName} (id_pk, c_varchar) VALUES (?, ?)`, [2, 'To be rolled back'])
  await db.rollback()
  console.log('    Rollback successful.')

  const result = await db.exec(`SELECT count(*) as count FROM ${testTableName} WHERE id_pk = 2`)
  assert.strictEqual(result[0].count, 0, 'Data should NOT be present after ROLLBACK.')
  console.log('    Data verified to be absent.')
  console.timeEnd('Rollback Duration')
}

async function testPreparedStatements(db) {
  console.time('Prepared Statements Duration')
  console.log('\n[6/10] Testing Prepared Statements...')
  const insertSQL = `INSERT INTO ${testTableName} (id_pk, c_varchar, c_integer) VALUES (?, ?, ?)`
  const stmt = await db.prepare(insertSQL)
  const stmtExec = stmt.exec.bind(stmt)
  const stmtDrop = stmt.drop.bind(stmt)

  await stmtExec([3, 'Prepared Statement', 123])
  await db.commit()
  console.log('    Prepared INSERT committed.')

  const result = await db.exec(`SELECT c_integer FROM ${testTableName} WHERE id_pk = 3`)
  assert.strictEqual(result[0].c_integer, 123, 'Prepared statement data mismatch.')
  console.log('    Prepared statement data verified.')

  await stmtDrop()
  console.log('    Statement dropped.')
  console.timeEnd('Prepared Statements Duration')
}

async function testCreateAndExecuteProcedures(db) {
  console.time('Create and Execute Procedures Duration')
  console.log(`\n[7/10] Creating and testing procedures...`)
  await db.exec(`
      CREATE PROCEDURE ${testProcName}(IN prod_id INT)
      RESULT (res_varchar VARCHAR(100), res_double DOUBLE)
      BEGIN SELECT c_varchar, c_double FROM ${testTableName} WHERE id_pk = prod_id; END
    `)
  await db.commit()
  console.log(`    Procedure '${testProcName}' created.`)

  let result = await db.exec(`CALL ${testProcName}(?)`, [1])
  assert.strictEqual(result[0].res_varchar, 'Variable character data', 'Procedure varchar mismatch.')
  assert.strictEqual(result[0].res_double, 1.23456789e10, 'Procedure double mismatch.')
  console.log('    SELECT procedure executed successfully.')

  await db.exec(`
        CREATE PROCEDURE ${updateProcName}(IN prod_id INT, IN new_name VARCHAR(100))
        BEGIN UPDATE ${testTableName} SET c_varchar = new_name WHERE id_pk = prod_id; END
    `)
  await db.commit()
  console.log(`    Procedure '${updateProcName}' created.`)

  await db.exec(`CALL ${updateProcName}(?, ?)`, [1, 'Updated First Entry'])
  await db.commit()

  result = await db.exec(`SELECT c_varchar FROM ${testTableName} WHERE id_pk = 1`)
  assert.strictEqual(result[0].c_varchar, 'Updated First Entry', 'Procedure data modification failed.')
  console.log('    UPDATE procedure executed successfully.')
  console.timeEnd('Create and Execute Procedures Duration')
}

async function testMultipleResultSets(db) {
  console.time('Multiple Result Sets Duration')
  console.log('\n[8/10] Testing multiple result sets...')
  await db.exec(`
        CREATE PROCEDURE ${multiResultProcName}()
        BEGIN
            SELECT id_pk, c_varchar FROM ${testTableName} WHERE id_pk = 1;
            SELECT id_pk, c_integer FROM ${testTableName} WHERE id_pk = 3;
        END
    `)
  await db.commit()
  console.log(`    Procedure '${multiResultProcName}' created.`)

  const stmt = await db.prepare(`CALL ${multiResultProcName}()`)
  const stmtExec = stmt.exec.bind(stmt)
  const getMoreResults = stmt.getMoreResults.bind(stmt)
  const stmtDrop = stmt.drop.bind(stmt)

  const firstResult = await stmtExec()
  assert.strictEqual(firstResult.length, 1, 'First result set should have one row.')
  assert.strictEqual(firstResult[0].c_varchar, 'Updated First Entry', 'First result set data mismatch.')
  console.log('    First result set verified.')

  const secondResult = await getMoreResults()
  assert.strictEqual(secondResult.length, 1, 'Second result set should have one row.')
  assert.strictEqual(secondResult[0].c_integer, 123, 'Second result set data mismatch.')
  console.log('    Second result set verified.')

  try {
    await getMoreResults()
    // If this line is reached, the test fails because an error was expected.
    assert.fail('Expected an error when fetching beyond the last result set, but none was thrown.')
  } catch (error) {
    // This is the expected outcome. We verify it's the correct informational message.
    assert.ok(error.message.includes('Procedure has completed'), `Expected "Procedure has completed" error, but got: ${error.message}`)
    console.log('    End of result sets confirmed with expected message.')
  }

  await stmtDrop()
  console.log('    Statement dropped.')
  console.timeEnd('Multiple Result Sets Duration')
}

async function testErrorHandling(db) {
  console.time('Error Handling Duration')
  console.log('\n[9/10] Testing Error Handling...')
  try {
    await db.exec('SELECT * FROM THIS_TABLE_DOES_NOT_EXIST')
    throw new Error('Query should have failed but it succeeded.')
  } catch (error) {
    assert.ok(error.message.includes('not found'), 'Error message should indicate table not found.')
    console.log('    Successfully caught expected error.')
  }
  await db.rollback()
  console.timeEnd('Error Handling Duration')
}

// --- Test Runner ---

async function runTests(db) {
  await testCreateTable(db)
  await testInsertAndCommit(db)
  await testRollback(db)
  await testPreparedStatements(db)
  await testCreateAndExecuteProcedures(db)
  await testMultipleResultSets(db)
  await testErrorHandling(db)
}

async function main() {
  console.time('Total Test Duration')
  const db = new sqlanywhere.createConnection()

  try {
    console.log('--- TEST SUITE START ---')
    console.time('Connection Duration')
    console.log('\n[1/10] Connecting to database...')
    await db.connect(connParams)
    console.log('    Connection successful!')
    console.timeEnd('Connection Duration')

    console.time('Cleanup Duration')
    console.log('\n[2/10] Cleaning up previous test objects...')
    await db.exec(`DROP PROCEDURE IF EXISTS ${testProcName}`)
    await db.exec(`DROP PROCEDURE IF EXISTS ${updateProcName}`)
    await db.exec(`DROP PROCEDURE IF EXISTS ${multiResultProcName}`)
    await db.exec(`DROP TABLE IF EXISTS ${testTableName}`)
    await db.commit()
    console.log('    Cleanup complete.')
    console.timeEnd('Cleanup Duration')

    await runTests(db)
  } catch (error) {
    console.error('\n--- A TEST FAILED ---')
    console.error(error)
  } finally {
    console.time('Disconnection Duration')
    console.log('\n[10/10] Disconnecting...')
    await db.disconnect()
    console.log('    Disconnected.')
    console.timeEnd('Disconnection Duration')
    console.log('\n--- TEST SUITE COMPLETE ---')
  }
  console.timeEnd('Total Test Duration')
}

main()
