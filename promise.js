'use strict';
const sqlanywhere = require('./index');
const util = require('util');

// Promisify the statement object
function promisifyStatement(stmt) {
  return {
    exec: util.promisify(stmt.exec).bind(stmt),
    drop: util.promisify(stmt.drop).bind(stmt),
    getMoreResults: util.promisify(stmt.getMoreResults).bind(stmt),
  };
}

// Promisify the connection object
function promisifyConnection(conn) {
  const promisified = {
    connect: util.promisify(conn.connect).bind(conn),
    disconnect: util.promisify(conn.disconnect).bind(conn),
    exec: util.promisify(conn.exec).bind(conn),
    commit: util.promisify(conn.commit).bind(conn),
    rollback: util.promisify(conn.rollback).bind(conn),
    connected: conn.connected.bind(conn), // This is a synchronous method

    // We need to wrap prepare to ensure it returns a promisified statement
    prepare: async (sql) => {
      const preparePromise = util.promisify(conn.prepare).bind(conn);
      const stmt = await preparePromise(sql);
      return promisifyStatement(stmt);
    },
  };
  // Add the 'close' alias for disconnect
  promisified.close = promisified.disconnect;
  return promisified;
}

// Export a new createConnection function that returns a promisified connection
function createPromisedConnection() {
    const conn = new sqlanywhere.Connection();
    return promisifyConnection(conn);
}

module.exports = {
    createConnection: createPromisedConnection,
};