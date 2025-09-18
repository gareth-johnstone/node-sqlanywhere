# @iqx-limited/sqlanywhere

A modern, high-performance Node.js driver for [SAP SQL Anywhere](https://www.sap.com/products/sql-anywhere.html).

This driver is built using N-API, ensuring a stable and future-proof interface that is independent of the underlying JavaScript engine. It is fully `async/await` compatible, providing a clean and modern developer experience.

[![NPM](https://nodei.co/npm/@iqx-limited/sqlanywhere.svg?compact=true)](https://nodei.co/npm/@iqx-limited/sqlanywheresqlanywhere/)

## Prerequisites

* **Node.js**: Version 18.0.0 or higher.
* **C++ Toolchain**: A C++ compiler and build tools are required for the native addon compilation. This is typically handled by `node-gyp`. Please see the [`node-gyp` installation guide](https://github.com/nodejs/node-gyp#installation) for platform-specific instructions.
* **SAP SQL Anywhere**: The appropriate SQL Anywhere client libraries must be installed and correctly configured in your environment (e.g., via the `$SQLANY` environment variable and system path).

## Installation

```sh
npm install @iqx-limited/sqlanywhere
```

The installation process will automatically compile the native C++ addon for your platform.

## Getting Started

The driver uses Promises for all asynchronous operations, making it ideal for use with `async/await`.

```javascript
const sqlanywhere = require('@iqx-limited/sqlanywhere');

// --- For security, use environment variables for credentials ---
const connParams = {
  ServerName: process.env.DB_SERVER || 'demo',
  UserID: process.env.DB_USER || 'DBA',
  Password: process.env.DB_PASSWORD || 'sql'
};

async function main() {
  const connection = sqlanywhere.createConnection();

  try {
    // 1. Connect to the database
    console.log('Connecting...');
    await connection.connect(connParams);
    console.log('Connection successful!');

    // 2. Execute a query with parameters
    const products = await connection.exec(
      'SELECT Name, Description FROM Products WHERE id = ?',
      [301]
    );
    console.log('Query Result:', products[0]);
    // output --> Query Result: { Name: 'Tee Shirt', Description: 'V-neck' }

  } catch (error) {
    console.error('An error occurred:', error);
  } finally {
    // 3. Disconnect when finished
    console.log('Disconnecting...');
    await connection.disconnect();
    console.log('Disconnected.');
  }
}

main();
```

## API Reference

All asynchronous methods return a `Promise`.

### Connection

`sqlanywhere.createConnection()`
Creates a new, uninitialized connection object.

`connection.connect(params)`
Establishes a connection to the database. The `params` object can contain most valid [SQL Anywhere connection properties](https://www.google.com/search?q=http://dcx.sap.com/index.html%23sa160/en/dbadmin/da-conparm.html).

`connection.disconnect()` or `connection.close()`
Closes the database connection.

`connection.exec(sql, [params])`
Executes a SQL statement directly.

* For `SELECT` queries, it returns a `Promise` that resolves to an array of result objects.
* For DML statements (`INSERT`, `UPDATE`, `DELETE`), it returns a `Promise` that resolves to the number of affected rows.
* Parameters can be bound using `?` placeholders.

`connection.prepare(sql)`
Prepares a SQL statement for later execution. Returns a `Promise` that resolves to a `Statement` object.

`connection.commit()`
Commits the current transaction.

`connection.rollback()`
Rolls back the current transaction.

### Statement (from `connection.prepare()`)

`statement.exec([params])`
Executes a prepared statement. The return value is the same as `connection.exec()`.

`statement.getMoreResults()`
For procedures or batches that return multiple result sets, this method advances to the next result set. Returns a `Promise` that resolves to the next array of results. When no more result sets are available, the promise will reject with a "Procedure has completed" message.

`statement.drop()`
Frees the resources associated with the prepared statement on the database server.

## Data Type Support

This driver provides comprehensive support for a wide range of SQL Anywhere data types, which are automatically mapped to the most appropriate JavaScript types:

| SQL Anywhere Type(s)                                                                       | JavaScript Type                               |
| ------------------------------------------------------------------------------------------ | --------------------------------------------- |
| `INTEGER`, `BIGINT`, `SMALLINT`, `TINYINT`, `DECIMAL`, `DOUBLE`, `FLOAT`, `BIT`, etc.       | `number`                                      |
| `VARCHAR`, `CHAR`, `LONG NVARCHAR`, `DATE`, `TIME`, `TIMESTAMP`, `UNIQUEIDENTIFIER`, `XML` | `string`                                      |
| `BINARY`, `VARBINARY`, `LONG BINARY`, `ST_GEOMETRY`                                        | `Buffer`                                      |

## Running the Test Suite

The project includes a comprehensive test suite that validates all driver functionality against a live database.

1. **Configure Credentials**: Copy the `examples/.env.sample` file to a new file named `examples/.env` and fill in your database connection details.

2. **Run Tests**: Execute the following command from the examples directory:

    ```sh
    npm run test
    ```

## Resources

* [SAP SQL Anywhere Documentation](http://dcx.sap.com/)
* [SAP SQL Anywhere Developer Q\&A Forum](http://sqlanywhere-forum.sap.com/)

<!-- end list -->