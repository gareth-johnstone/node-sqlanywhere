export interface ConnectionParams {
  ServerName?: string;
  UserID?: string;
  Password?: string;
  [key: string]: string | undefined;
}

export type QueryValue = string | number | Buffer | null;
export type QueryParams = QueryValue[];
export type QueryResult = Record<string, any>[];

export class Statement {
    /**
     * Executes a prepared statement.
     * @param params Optional array of parameters for the statement.
     * @param callback Callback function.
     */
    exec(params?: QueryParams, callback?: (err: Error | null, result?: QueryResult | number) => void): void;
    exec(callback: (err: Error | null, result?: QueryResult | number) => void): void;

    /**
     * For procedures that return multiple result sets, this method advances to the next result set.
     * @param callback Callback function.
     */
    getMoreResults(callback: (err: Error | null, result?: QueryResult) => void): void;

    /**
     * Frees the resources associated with the prepared statement.
     * @param callback Callback function.
     */
    drop(callback: (err: Error | null) => void): void;
}

export class Connection {
    constructor();

    /**
     * Establishes a connection to the database.
     * @param params Connection parameters.
     * @param callback Callback function.
     */
    connect(params: ConnectionParams, callback: (err: Error | null) => void): void;

    /**
     * Closes the database connection.
     * @param callback Callback function.
     */
    disconnect(callback: (err: Error | null) => void): void;
    close(callback: (err: Error | null) => void): void;

    /**
     * Executes a SQL statement.
     * @param sql The SQL statement to execute.
     * @param params Optional array of parameters.
     * @param callback Callback function.
     */
    exec(sql: string, params?: QueryParams, callback?: (err: Error | null, result?: QueryResult | number) => void): void;
    exec(sql: string, callback: (err: Error | null, result?: QueryResult | number) => void): void;

    /**
     * Prepares a SQL statement for later execution.
     * @param sql The SQL statement to prepare.
     * @param callback Callback function.
     */
    prepare(sql: string, callback: (err: Error | null, stmt?: Statement) => void): void;

    /**
     * Commits the current transaction.
     * @param callback Callback function.
     */
    commit(callback: (err: Error | null) => void): void;

    /**
     * Rolls back the current transaction.
     * @param callback Callback function.
     */
    rollback(callback: (err: Error | null) => void): void;

    /**
     * Checks if the connection is established.
     * @returns `true` if connected, otherwise `false`.
     */
    connected(): boolean;
}

/**
 * Creates a new Connection object.
 * @returns A new Connection instance.
 */
export class createConnection extends Connection {}