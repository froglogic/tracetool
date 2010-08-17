CREATE TABLE trace_entry (id INTEGER PRIMARY KEY AUTOINCREMENT,
                          pid INTEGER,
                          tid INTEGER,
                          timestamp DATETIME,
                          tracepoint_id INTEGER,
                          message TEXT);
CREATE TABLE trace_point (id INTEGER PRIMARY KEY AUTOINCREMENT,
                          verbosity INTEGER,
                          type INTEGER,
                          path_id INTEGER,
                          line INTEGER,
                          function_id INTEGER);
CREATE TABLE function_name (id INTEGER PRIMARY KEY AUTOINCREMENT,
                            name TEXT);
CREATE TABLE path_name (id INTEGER PRIMARY KEY AUTOINCREMENT,
                        name TEXT);
CREATE TABLE variable_value (tracepoint_id INTEGER,
                             name TEXT,
                             value TEXT,
                             UNIQUE (tracepoint_id, name));
CREATE TABLE backtrace (tracepoint_id INTEGER,
                        line INTEGER,
                        text TEXT);
