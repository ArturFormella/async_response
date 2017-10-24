#include <stdio.h>
#include "postgres.h"
#include "fmgr.h"
#include <hiredis/hiredis.h>
#include <hiredis/async.h>

#include "utils/builtins.h"

#define DEFAULTSOCKET "/var/run/redis/redis.sock"
#define DEFAULTPORT 6379
#define DEFAULTHOST "127.0.0.1"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1( async_response_tcp ); 			// (integer, text, text, text)
PG_FUNCTION_INFO_V1( async_response_socket ); 		// (text, text, text, text)

bool true_send( redisContext *ctx, char *channel, char *aspect, char *msg);

Datum async_response_tcp(PG_FUNCTION_ARGS) { // (integer, text, text, text)
  struct timeval timeout = { 0, 500000 }; /* 0.5 minutes */
  int32 port = PG_GETARG_INT32(0); 
  char *channel = text_to_cstring(PG_GETARG_TEXT_P(1));
  char *aspect = text_to_cstring(PG_GETARG_TEXT_P(2));
  char *msg = PG_ARGISNULL(3) ? NULL : text_to_cstring(PG_GETARG_TEXT_P(3));
  redisContext *ctx = redisConnectWithTimeout(DEFAULTHOST, port, timeout);
  return true_send( ctx, channel, aspect, msg);
}

Datum async_response_socket(PG_FUNCTION_ARGS) { // (text, text, text, text)
  struct timeval timeout = { 0, 500000 }; /* 0.5 minutes */
  char *socket = text_to_cstring(PG_GETARG_TEXT_P(0));
  char *channel = text_to_cstring(PG_GETARG_TEXT_P(1));
  char *aspect = text_to_cstring(PG_GETARG_TEXT_P(2));
  char *msg = PG_ARGISNULL(3) ? NULL : text_to_cstring(PG_GETARG_TEXT_P(3));
  redisContext *ctx = redisConnectUnixWithTimeout(socket, timeout);
  return true_send( ctx, channel, aspect, msg);
}

bool true_send( redisContext *ctx, char *channel, char *aspect, char *msg){
	redisReply *reply = NULL;
	if (ctx == NULL || ctx->err) {
		if (ctx) {
			ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("failed to connect to redis: %s", ctx->errstr)));
			redisFree(ctx);
		} else {
			ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("failed to connect to redis: can't allocate redis context")));
		}
		PG_RETURN_BOOL(false);
	}
  if (msg == NULL) {
    reply = redisCommand(ctx, "PUBLISH %s %s~NULL", channel, aspect );
  }else{
    reply = redisCommand(ctx, "PUBLISH %s %s~%s", channel, aspect, msg );
  }
	freeReplyObject(reply);
	if (ctx->err) {
		ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("failed to PUBLISH to redis: %s", ctx->errstr)));
		redisFree(ctx);
		PG_RETURN_BOOL(false);
	} else {
		redisFree(ctx);
		PG_RETURN_BOOL(true);
	}
}
/*
PG_FUNCTION_INFO_V1( async_response_async );  		// (integer, text, text, text)

Datum async_response_async(PG_FUNCTION_ARGS) { // (integer, text, text)
  int ret;
  
  //  int port;
  //  char *host;

  // struct event_base *base = event_base_new();
  // int32 port = PG_GETARG_INT32(0);
  // char *channel = text_to_cstring(PG_GETARG_TEXT_P(1));
  // char *msg = text_to_cstring(PG_GETARG_TEXT_P(2));

	// host = GetConfigOptionByName("async_response.host", NULL);
	// port = atoi(GetConfigOptionByName("async_response.port", NULL));

  redisAsyncContext *ctx = redisAsyncConnect("127.0.0.1", 6379);
  if (ctx == NULL || ctx->err) {
    if (ctx) {
      ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("failed to connect to redis: %s", ctx->errstr)));
      redisAsyncFree(ctx);
    } else {
      ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("failed to connect to redis: can't allocate redis context")));
    }
    PG_RETURN_BOOL(false);
  }

  redisLibevAttach(EV_DEFAULT_ ctx);
  redisAsyncCommand(ctx, NULL, NULL, "SET key 123123");
  ret = redisAsyncCommand(ctx, NULL, NULL, "PUBLISH seat 123123123");
  ev_loop(EV_DEFAULT_ 0);
  if (REDIS_ERR == ret){
   //  redisAsyncFree(ctx);
     PG_RETURN_BOOL(false);
  }
  if (ctx->err) {
    ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("failed to PUBLISH to redis: %s", ctx->errstr)));
  //  redisAsyncFree(ctx);
    PG_RETURN_BOOL(false);
  } else {
  //  redisAsyncFree(ctx);
    PG_RETURN_BOOL(true);
  }
}
*/
