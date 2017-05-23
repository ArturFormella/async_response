
MODULE_big = async_response
OBJS = async_response.o
DATA = async_response--1.0.sql

EXTENSION = async_response

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

SHLIB_LINK += -lhiredis
