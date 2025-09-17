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

const util = require('util')
const assert = require('assert')
// Load the compiled addon directly
const sqlanywhere = require('../build/Release/sqlanywhere.node')

// --- IMPORTANT ---
// For security, load database credentials from environment variables.
// You can set these in your shell or in a .env file (excluded from version control).
// Example (in bash): 
//   export DB_SERVER=your_server
//   export DB_USER=your_user
//   export DB_PASSWORD=your_password
// Never commit real credentials to source code or version control!
const connParams = {
  ServerName: process.env.DB_SERVER || 'database', // Your database server name
  UserID: process.env.DB_USER || 'username',       // Your user ID
  Password: process.env.DB_PASSWORD || 'password'  // Your password
}

// Create a new connection object
const client = new sqlanywhere.createConnection()

// --- Promisify the client and statement methods for async/await ---
const db = {
  connect: util.promisify(client.connect).bind(client),
  disconnect: util.promisify(client.disconnect).bind(client),
  exec: util.promisify(client.exec).bind(client),
  prepare: util.promisify(client.prepare).bind(client),
  commit: util.promisify(client.commit).bind(client),
  rollback: util.promisify(client.rollback).bind(client)
}

// Main async function to run all tests
async function runAllTests() {
  const testTableName = 'NAPI_DRIVER_TEST'

  try {
    console.log('--- TEST SUITE START ---')
    console.log('\n[1/7] Connecting to database...')
    await db.connect(connParams)
    console.log('    Connection successful!')

    // --- Test 1: DDL and Parameterized INSERT + COMMIT ---
    console.log(`\n[2/7] Creating test table '${testTableName}'...`)
    await db.exec(`CREATE TABLE ${testTableName} (id INT PRIMARY KEY, name VARCHAR(100), value DOUBLE)`)
    console.log('    Table created.')

    console.log('    Inserting data with parameter binding...')
    const insertSQL = `INSERT INTO ${testTableName} (id, name, value) VALUES (?, ?, ?)`
    const insertParams = [1, 'First Entry', 123.45]
    let affectedRows = await db.exec(insertSQL, insertParams)
    assert.strictEqual(affectedRows, 1, 'First INSERT should affect 1 row.')
    console.log('    Data inserted. Committing transaction...')
    await db.commit()
    console.log('    Commit successful.')

    let result = await db.exec(`SELECT name FROM ${testTableName} WHERE id = 1`)
    assert.strictEqual(result[0].name, 'First Entry', 'Data should be present after COMMIT.')
    console.log('    Data verified after commit.')

    // --- Test 2: ROLLBACK ---
    console.log(`\n[3/7] Testing ROLLBACK...`)
    console.log('    Inserting temporary data...')
    await db.exec(insertSQL, [2, 'Second Entry (to be rolled back)', 678.9])
    console.log('    Rolling back transaction...')
    await db.rollback()
    console.log('    Rollback successful.')

    result = await db.exec(`SELECT count(*) as count FROM ${testTableName} WHERE id = 2`)
    assert.strictEqual(result[0].count, 0, 'Data should NOT be present after ROLLBACK.')
    console.log('    Data verified to be absent after rollback.')

    // --- Test 3: Prepared Statements ---
    console.log('\n[4/7] Testing Prepared Statements...')
    const stmt = await db.prepare(insertSQL)
    const stmtExec = util.promisify(stmt.exec).bind(stmt)
    const stmtDrop = util.promisify(stmt.drop).bind(stmt)

    console.log('    Executing prepared INSERT...')
    affectedRows = await stmtExec([3, 'Prepared Statement Entry', 99.0])
    assert.strictEqual(affectedRows, 1, 'Prepared INSERT should affect 1 row.')
    await db.commit()
    console.log('    Prepared INSERT committed.')

    result = await db.exec(`SELECT value FROM ${testTableName} WHERE id = 3`)
    assert.strictEqual(result[0].value, 99.0, 'Prepared statement data should be present.')
    console.log('    Prepared statement data verified.')

    console.log('    Dropping prepared statement...')
    await stmtDrop()
    console.log('    Statement dropped.')

    // --- Test 4: Error Handling ---
    console.log('\n[5/7] Testing Error Handling...')
    try {
      await db.exec('SELECT * FROM THIS_TABLE_DOES_NOT_EXIST')
      // If we get here, the test fails
      throw new Error('Query should have failed but it succeeded.')
    } catch (error) {
      assert.ok(error.message.includes('not found'), 'Error message should indicate table not found.')
      console.log('    Successfully caught expected error from invalid query.')
    }
    await db.rollback() // Clear any error state from the connection
  } catch (error) {
    console.error('\n--- A TEST FAILED ---')
    console.error(error)
  } finally {
    // --- Cleanup ---
    console.log('\n[6/7] Cleaning up test table...')
    try {
      await db.exec(`DROP TABLE ${testTableName}`)
      await db.commit()
      console.log(`    Table '${testTableName}' dropped.`)
    } catch (cleanupError) {
      console.warn('    Warning: Could not drop test table. It may not exist.', cleanupError.message)
    }

    console.log('\n[7/7] Disconnecting...')
    await db.disconnect()
    console.log('    Disconnected.')
    console.log('\n--- TEST SUITE COMPLETE ---')
  }
}

// Start the tests
runAllTests()
