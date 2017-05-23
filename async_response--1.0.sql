
DROP FUNCTION IF EXISTS async_response(INTEGER,TEXT,TEXT,TEXT);
DROP FUNCTION IF EXISTS async_response(TEXT,TEXT,TEXT,TEXT);
/*
DROP FUNCTION IF EXISTS async_response_async(INTEGER,TEXT,TEXT,TEXT);
*/

CREATE FUNCTION async_response(INTEGER,TEXT,TEXT,TEXT) 			RETURNS boolean AS 'async_response', 'async_response_tcp' LANGUAGE C STABLE STRICT;
CREATE FUNCTION async_response(TEXT,TEXT,TEXT,TEXT) 			RETURNS boolean AS 'async_response', 'async_response_socket' LANGUAGE C STABLE STRICT;
/*
CREATE FUNCTION async_response_async(INTEGER,TEXT,TEXT,TEXT)	RETURNS boolean AS 'async_response', 'async_response_async' LANGUAGE C STABLE STRICT;
*/
