#include <string.h>
#include "iRepository.h"
#include "private.h"


bool _dbc_t::connect(_cstr_t db_host, _cstr_t db_port, _cstr_t db_user, _cstr_t db_pass, _cstr_t db_name) {
	bool r = false;

	mp_pgres = NULL;

	if(!mp_pgc) {
		if((mp_pgc = PQsetdbLogin(db_host, db_port, NULL, NULL, db_name, db_user, db_pass)))
			r = true;
	} else
		r = true;

	return r;
}

void _dbc_t::disconnect(void) {
	if(mp_pgres) {
		PQclear(mp_pgres);
		mp_pgres = NULL;
	}
	if(mp_pgc) {
		PQfinish(mp_pgc);
		mp_pgc = NULL;
	}
}

ConnStatusType _dbc_t::status(void) {
	ConnStatusType r = CONNECTION_BAD;

	if(mp_pgc)
		r = PQstatus(mp_pgc);

	return r;
}

_cstr_t _dbc_t::error_text(void) {
	_cstr_t r = "Unknown error";

	if(mp_pgc)
		r = PQerrorMessage(mp_pgc);

	return r;
}

_cstr_t _dbc_t::result_error_text(void) {
	_cstr_t r = "Unknown error";

	if(mp_pgres)
		r = PQresultErrorMessage(mp_pgres);

	return r;
}

void _dbc_t::clear(void) {
	if(mp_pgres) {
		PQclear(mp_pgres);
		mp_pgres = NULL;
	}
}

bool _dbc_t::exec(_cstr_t query) {
	bool r = false;

	if(mp_pgc) {
		ExecStatusType res;

		clear();
		mp_pgres = PQexec(mp_pgc, query);
		res = PQresultStatus(mp_pgres);

		if(res == PGRES_COMMAND_OK || res == PGRES_TUPLES_OK)
			r = true;
	}

	return r;
}

_u32 _dbc_t::fields(void) {
	_u32 r = 0;

	if(mp_pgres)
		r = PQnfields(mp_pgres);

	return r;
}

_u32 _dbc_t::rows(void) {
	_u32 r = 0;

	if(mp_pgres)
		r = PQntuples(mp_pgres);

	return r;
}

_cstr_t _dbc_t::value(_u32 row, _u32 field) {
	_cstr_t r = NULL;

	if(mp_pgres)
		r = PQgetvalue(mp_pgres, row, field);

	return r;
}

_u32 _dbc_t::length(_u32 row, _u32 field) {
	_u32 r = 0;

	if(mp_pgres)
		r = PQgetlength(mp_pgres, row, field);

	return r;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

dbc_incubator::dbc_incubator() {
	mpi_pool = NULL;
	m_db_host = m_db_port = m_db_user = m_db_pass = m_db_name = NULL;
}

dbc_incubator::~dbc_incubator() {
	destroy();
}

void dbc_incubator::init(_cstr_t db_host, _cstr_t db_port, _cstr_t db_user, _cstr_t db_pass, _cstr_t db_name) {
	if(!mpi_pool) {
		m_db_host = db_host;
		m_db_port = db_port;
		m_db_user = db_user;
		m_db_pass = db_pass;
		m_db_name = db_name;

		if((mpi_pool = dynamic_cast<iPool *>(_gpi_repo_->object_by_iname(I_POOL, RF_CLONE)))) {
			mpi_pool->init(sizeof(_dbc_t), [](_u8 op, void *data, void *udata) {
				_dbc_t *p_dbc = (_dbc_t *)data;
				_dbc_incubator_t *p_dbci = (_dbc_incubator_t *)udata;

				switch(op) {
					case POOL_OP_NEW:
					case POOL_OP_BUSY:
						p_dbc->connect(p_dbci->m_db_host,
								p_dbci->m_db_port,
								p_dbci->m_db_user,
								p_dbci->m_db_pass,
								p_dbci->m_db_name);
						break;
					case POOL_OP_FREE:
						p_dbc->clear();
						break;
					case POOL_OP_DELETE:
						p_dbc->disconnect();
						break;
				}
			}, this);
		}

		mpi_log = dynamic_cast<iLog *>(_gpi_repo_->object_by_iname(I_LOG, RF_ORIGINAL));
	}
}

_dbc_t *dbc_incubator::alloc(void) {
	_dbc_t *r = (_dbc_t *)mpi_pool->alloc();

	if(r) {
		if(r->status() != CONNECTION_OK) {
			mpi_log->fwrite(LMT_ERROR, "DB: %s", r->error_text());
			r->disconnect();
			free(r);
			r = NULL;
		}
	}

	return r;
}

void dbc_incubator::free(_dbc_t *p_dbc) {
	if(p_dbc)
		mpi_pool->free(p_dbc);
}

void dbc_incubator::destroy(void) {
	if(mpi_pool) {
		_gpi_repo_->object_release(mpi_pool);
		mpi_pool = NULL;
		_gpi_repo_->object_release(mpi_log);
		mpi_log = NULL;
	}
}

