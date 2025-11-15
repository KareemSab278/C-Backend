# C Backend Project

## Overview

This project is a custom-built backend server written entirely in C (gigachad language), designed to provide a deep understanding of how backend systems work without relying on high-level frameworks like Node.js or FastAPI (ew). The goal was to explore the "under the hood" mechanics of web servers, HTTP handling, database interactions, and API endpoints. By building everything from scratch (almost lol im not building a whole db program), I've gained insight into the abstractions that frameworks provide and the complexities they hide.

The server uses SQLite as the database and handles HTTP POST requests to execute specific "function calls" (acting as endpoints). I went with sqlite3 because it is based and mySQL for c had lacking or misleading documentation, (you suck, Oracle). It's not a RESTful API in the traditional sense; instead, it processes POST data to determine which function to run, making it a unique (and slightly dumb) implementation.

This project is for learning raw C programming for web development - yes, there were even times I decided to reinvent the wheel even though there were some options for libs online such as https://lwan.ws/ and https://facil.io/ (thanks, Linguistic-mystic from Reddit!). It's not intended for production use but serves as a foundation for understanding low-level backend stuff. I might come back to this code one day and find it useful, or horrible. Until then, I get to brag about building this project.

(also i still dont get pointers...)

## Features

- **HTTP Server**: Custom-built server using Winsock on Windows or POSIX sockets on Linux. There are comments in some files for linux or windows users (we dont talk about mac).
- **Database Integration**: SQLite for db.
- **Function-Based Endpoints**: Instead of traditional REST endpoints, the server processes POST requests with a `function` parameter to call specific C functions.
- **JSON Responses**: All responses are in JSON format. Too lazy to JSONify in frontend. Also faster in C.
- **Frontend Demo**: A simple HTML/JS frontend to interact with the server.
- **Modular Structure**: Code is organised into separate files for maintainability.
- **Functional Programming**: The correct way to write code. (OOP sucks deal with it).

## File Structure

The project is structured to separate concerns, making it easier to understand and maintain. Here's a breakdown:

### Root Directory
- **`main.c`**: The entry point of the application. It simply calls the `server()` function from `server.c` to start the server loop.
- **`server.c`**: Contains the core server logic, including socket initialization, HTTP request parsing, and function call handling. This is the heart of the backend.
- **`server.h`**: Header file declaring the `server()` function prototype.
- **`sqlite3.c` and `sqlite3.h`**: The SQLite library source code. This is included directly rather than linking to a system library for portability and to avoid dependencies.

### DBFunctions/ Directory
- **`DB.c`**: Handles all database operations. It includes functions to initialize the database, check if it exists, and execute SQL queries. This abstraction layer makes database interactions reusable.
- **`DB.h`**: Header file declaring database-related function prototypes.

### Frontend/ Directory
- **`index.html`**: A simple HTML page with a button to fetch records and display them. It includes a script tag for `index.js`.
- **`index.js`**: JavaScript code that makes AJAX requests to the server. It fetches records on page load and provides a delete function for each record.

### helpers/ Directory
- **`escape_JSON.c`**: Contains a utility function to escape special characters in strings for safe JSON output (e.g., escaping quotes and newlines).
- **`helpers.h`**: Header file declaring the `escape_JSON` function prototype.

### Why This Structure?
- **Modularity**: Each file has a single responsibility. For example, `server.c` handles networking, `DB.c` handles database logic, and `helpers/` contains utility functions. This makes the code easier to debug and extend.
- **Separation of Concerns**: Database logic is isolated in `DBFunctions/`, frontend in `Frontend/`, and utilities in `helpers/`. This prevents code from becoming monolithic.
- **Header Files**: Using `.h` files allows for function declarations without implementations, enabling better compilation and avoiding circular dependencies.
- **Inclusion of SQLite Source**: By including `sqlite3.c` directly, the project is self-contained and doesn't require external SQLite installations, which is useful for learning and portability.
- **Frontend Separation**: The frontend is in its own directory to simulate a client-server architecture, even though it's served statically here.

## How to Run

### Prerequisites
- **GCC Compiler**: Ensure you have GCC installed on your system.
- **Windows**: No additional libraries needed beyond what's included.
- **Linux**: Ensure you have the necessary libs available.

### Compilation and Execution

#### On Windows
Run the following command in the project root directory:
```
gcc server.c main.c sqlite3.c DBFunctions/DB.c helpers/escape_JSON.c -o backend -lws2_32 -I.
```
- `-lws2_32`: Links the Winsock library for socket operations on Windows.
- `-I.`: Includes the current directory for header files.

Then, execute it:
```
./backend
```

#### On Linux (I use Mint)
Run the following command in the project root directory:
```
gcc server.c main.c sqlite3.c DBFunctions/DB.c helpers/escape_JSON.c -o backend -lpthread -ldl -lm -I.
```
- `-lpthread`: Links the POSIX threads library.
- `-ldl`: Links the dynamic linking library.
- `-lm`: Links the math library.
- `-I.`: Includes the current directory for header files.

Then, execute the binary:
```
./backend
```

The server will start listening on `http://localhost:6969`. You can access it via a browser, curl, or tools like Postman.

### Troubleshooting
- **Port Issues**: If the port doesn't close properly, on Linux use: `sudo netstat -tulnp | grep ':6969'` to find the process, then kill it.
- **Compilation Errors**: Ensure all files are in the correct directories and paths are absolute if needed.

## API Endpoints (Function Calls)

This project doesn't use traditional REST endpoints. Instead, all interactions are via HTTP POST requests to the root URL (`http://localhost:6969`). The POST body contains a `function` parameter that specifies which C function to execute. Parameters are passed via a `parameters` field.

### Available Functions

1. **`get_all_records`**
   - **Description**: Retrieves all records from the `SQL_IN_C` table in the database.
   - **POST Body**: `function=get_all_records`
   - **Response**: JSON array of objects, each with `id`, `name`, and `job` fields. Example: `[{"id":1,"name":"John","job":"Developer"}]`
   - **Usage in Code**: Calls `runSQL("SQL_IN_C.db", "SELECT * FROM SQL_IN_C;");` and returns the JSON result.

2. **`add_record`**
   - **Description**: Adds a new record to the `SQL_IN_C` table.
   - **POST Body**: `function=add_record&parameters='name', 'job'` (Note: Parameters must be properly formatted as SQL values, e.g., `'jacob', 'manager'`)
   - **Response**: JSON object indicating success or failure, e.g., `{"status":"success","message":"Query executed successfully","affected_rows":1}`
   - **Usage in Code**: Constructs an INSERT query like `INSERT INTO SQL_IN_C (name, job) VALUES ('jacob', 'manager');` and executes it.

3. **`delete_record`**
   - **Description**: Deletes a record from the `SQL_IN_C` table by ID.
   - **POST Body**: `function=delete_record&parameters=id` (e.g., `function=delete_record&parameters=4`)
   - **Response**: JSON object indicating success or failure.
   - **Usage in Code**: Constructs a DELETE query like `DELETE FROM SQL_IN_C WHERE id=4;` and executes it.

### How It Works
- The server parses the POST body to extract `function` and `parameters`.
- It calls `runFrontendFunctionCall(function_call, parameters)` in `server.c`, which maps the function name to the appropriate C function.
- Each function uses `runSQL()` from `DB.c` to interact with the SQLite database.
- Responses are formatted as JSON using the database results or success messages.

### Testing with Tools
- **Curl**: `curl -X POST -d "function=get_all_records" http://localhost:6969`
- **Postman**: Send a POST request to `http://localhost:6969` with body `function=add_record&parameters='kareem', 'workplace legend'`
- **Browser**: Open `http://localhost:6969/Frontend/index.html` for the demo frontend.

## Detailed Function Explanations

### Server Functions (server.c)
- **`populateResponseBody`**: Sets the response structure with status code, text, content type, and body. Used to prepare HTTP responses.
- **`getHTTPRequestLine`**: Extracts the first line of an HTTP request (e.g., "POST / HTTP/1.1").
- **`getPostParam`**: Parses POST body parameters, extracting values for keys like "function" or "parameters".
- **`RETURN_API_OUTPUT`**: A wrapper for `runSQL`, used internally for API outputs.
- **`runFrontendFunctionCall`**: The main dispatcher. Takes the function name and parameters, executes the corresponding logic, and returns JSON.
- **`server`**: The main server loop. Initializes sockets, listens for connections, parses requests, calls functions, and sends responses.

### Database Functions (DBFunctions/DB.c)
- **`doesDatabaseExist`**: Checks if the database file exists; if not, creates it. Returns true/false.
- **`runSQL`**: Executes any SQL query. For SELECT, returns JSON array; for others, returns success message with affected rows.
- **`initializeDB`**: Calls `doesDatabaseExist` and creates the table if needed.

### Helper Functions (helpers/escape_JSON.c)
- **`escape_JSON`**: Escapes special characters in strings (quotes, newlines, backslashes) to make them safe for JSON output.

### Frontend Functions (Frontend/index.js)
- **`callAPI`**: Makes a POST request to the server with the given payload.
- **`fetchrecords`**: Fetches all records and updates the DOM with them, including delete buttons.
- **`deleteRecord`**: Prompts for confirmation and deletes a record by ID, then refreshes the list.

## Future Plans

- **Update and Create Users**: Add functions for updating existing records and creating new ones via the frontend.
- **More Endpoints**: Expand function calls for more CRUD operations.
- **Security**: Add input validation and sanitization.
- **Error Handling**: Improve error responses and logging.
- **PHP Integration**: As mentioned, integrate with a PHP frontend for curl requests.

This project has been an eye-opening experience in understanding the foundations of web backends. Contributions or suggestions are welcome, but remember, it's a learning process and I suck at C!