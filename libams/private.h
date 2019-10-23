#ifndef __AMS_PRIVATE_H__
#define __AMS_PRIVATE_H__

#include "libpq-fe.h"
#include "iMemory.h"
#include "iLog.h"

typedef struct dbc_incubator	_dbc_incubator_t;
typedef struct {
private:
	PGconn *mp_pgc;

public:
	bool connect(_cstr_t db_host, _cstr_t db_port, _cstr_t db_user, _cstr_t db_pass, _cstr_t db_name);
	void disconnect(void);
	ConnStatusType status(void);
	_cstr_t error_text(void);
}_dbc_t;

struct dbc_incubator {
private:
	iPool	*mpi_pool;
	iLog	*mpi_log;
	_cstr_t m_db_host;
	_cstr_t m_db_port;
	_cstr_t m_db_user;
	_cstr_t m_db_pass;
	_cstr_t m_db_name;

public:
	dbc_incubator();
	~dbc_incubator();

	void init(_cstr_t db_host, _cstr_t db_port, _cstr_t db_user, _cstr_t db_pass, _cstr_t db_name);
	_dbc_t *alloc(void);
	void free(_dbc_t *p_dbc);
	void destroy(void);
};

_dbc_incubator_t *get_dbci(void);

#endif
