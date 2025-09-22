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
     * @return `Promise<QueryResult | number>`
     */
    exec(params?: QueryParams): Promise<QueryResult | number>;
    exec(): Promise<QueryResult | number>;

    /**
     * For procedures that return multiple result sets, this method advances to the next result set.
     * @returns `Promise<QueryResult>`
     */
    getMoreResults(): Promise<QueryResult>;

    /**
     * Frees the resources associated with the prepared statement.
     * @returns `Promise<void>`
     */
    drop(): Promise<void>;
}

export class Connection {
    constructor();

    /**
     * Establishes a connection to the database.
     * @param params Connection parameters.
     * @returns `Promise<void>`
     */
    connect(params: ConnectionParams): Promise<void>;

    /**
     * Closes the database connection.
     * @returns `Promise<void>`
     */
    disconnect(): Promise<void>;
    close(): Promise<void>;

    /**
     * Executes a SQL statement.
     * @param sql The SQL statement to execute.
     * @param params Optional array of parameters.
     * @returns `Promise<QueryResult | number>`
     */
    exec(sql: string, params?: QueryParams): Promise<QueryResult | number>;
    exec(sql: string): Promise<QueryResult | number>;

    /**
     * Prepares a SQL statement for later execution.
     * @param sql The SQL statement to prepare.
     * @returns `Promise<Statement>`
     */
    prepare(sql: string): Promise<Statement>;

    /**
     * Commits the current transaction.
     * @returns `Promise<void>`
     */
    commit(): Promise<void>;

    /**
     * Rolls back the current transaction.
     * @returns `Promise<void>`
     */
    rollback(): Promise<void>;

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