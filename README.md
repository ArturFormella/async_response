# Deprecated in favor of [notify_now](https://github.com/ArturFormella/notify_now)

# ~~async_response~~
Call

    boolean async_response(socket TEXT, channel TEXT, aspect TEXT, data TEXT)
or

    boolean async_response(port INTEGER, channel TEXT, aspect TEXT, data TEXT)
in your complex query to send immediately to REDIS from PostgreSQL:

    PUBLISH channel message

http://redis.io/commands/publish

(message = aspect~data)

## Example

	select async_response(6379,'some_channel'::text,'aspect'::text,'dane'::text);
	
	select async_response('/var/run/redis/redis.sock','some_channel'::text,'products'::text,'{"a":32}'::text);
	select async_response('/var/run/redis/redis.sock','some_channel'::text,'categories'::text,'{"b":66}'::text);

## Real world example:

    WITH all_results as(

      /* This is my context. I don't want to send it back but I need it to get other data */

      SELECT
        num as id,
        ((num-1)%10) as cat_id,
        ((num-1)%5) as parent_id
      FROM generate_series(1,1070) as ser(num), pg_sleep(2)	/* its expensive */

    ), latest_id as (

      select max(id)  as max_id 
      from all_results

    ), request_latest_id as (

      /* send it back as soon as possible */
      SELECT async_response('/var/run/redis/redis.sock','some_channel','latest_id'::text, 
      (select max_id::text from latest_id))

    ), cats as (
      /* faceted search - 1000 ms */
      SELECT 
        cat_id, 
        count(1) as counter
      from 
        all_results, pg_sleep(1)
      group by cat_id

    ), request_cats as (

      SELECT async_response('/var/run/redis/redis.sock','some_channel','cats'::text, 
      (select json_agg( row_to_json(cats))::text from cats))

    ), parents as (

      /* faceted search - 1000 ms */
      SELECT 
        parent_id, 
        count(1) as counter
      from 
        all_results, pg_sleep(1)
      group by parent_id

    ), request_parents as (

      SELECT async_response('/var/run/redis/redis.sock','some_channel','parents'::text, 
      (select json_agg( row_to_json(parents))::text from parents))

    )

    SELECT 
      (select * from request_latest_id), /* you need at least touch request_latest_id to send it back */
      (select * from request_cats),
      (select * from request_parents)


Redis response (redis-cli):

    >SUBSCRIBE some_channel

    (+2002ms)
    1) "message"
    2) "some_channel"
    3) "latest_id~1070"
    
    (+1000ms)
    1) "message"
    2) "some_channel"
    3) "cats~[{\"cat_id\":7,\"counter\":107}, {\"cat_id\":2,\"counter\":107}, {\"cat_id\":6,\"counter\":107}, {\"cat_id\":9,\"counter\":107}, {\"cat_id\":0,\"counter\":107}, {\"cat_id\":3,\"counter\":107}, {\"cat_id\":5,\"counter\":107}, {\"cat_id\":1,\"counter\":107}, {\"cat_id\":4,\"counter\":107}, {\"cat_id\":8,\"counter\":107}]"
    
    (+1000ms)
    1) "message"
    2) "some_channel"
    3) "parents~[{\"parent_id\":2,\"counter\":214}, {\"parent_id\":0,\"counter\":214}, {\"parent_id\":3,\"counter\":214}, {\"parent_id\":1,\"counter\":214}, {\"parent_id\":4,\"counter\":214}]"


PostgresQL response:

    async_response	async_response	async_response
    true            true             true

    Time: 4,011.548 ms.

Advantages:
- first part of response is available after 2000ms, next after 3000ms
- you dont have to download the whole "all_results" or query it more than once
- you can do everything in one just query and have first-byte of response before the whole response is ready

## Installation

Requires [libhiredis-devel](https://github.com/redis/hiredis) package.

    $ make
    $ sudo make install
	
	drop extension if exists "async_response";
    create extension if not exists "async_response" with schema public;

    select * from async_response( 'unix_socket', 'channel', 'aspect', 'data');
    select * from async_response( port, 'channel', 'aspect', 'data');

Note: Unix_socket connection is a bit faster than tcp connection

  
