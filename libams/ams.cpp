#include "startup.h"
#include "iGatn.h"
#include "iArgs.h"
#include "iLog.h"
#include "private.h"

IMPLEMENT_BASE_ARRAY("libams", 10);

typedef struct {
	_u8 method;
	_cstr_t path;
	_gatn_route_event_t *handler;
}_route_handlers_t;

class cAms: public iGatnExtension {
private:
	iArgs	*mpi_args;
	iLog	*mpi_log;
	bool	m_use_doc_root;
	_cstr_t m_ams_path;
	_cstr_t m_db_host;
	_cstr_t m_db_port;
	_cstr_t m_db_user;
	_cstr_t m_db_pass;
	_cstr_t m_db_name;
	_dbc_incubator_t m_dbci;

	_route_handlers_t m_route[16]={
		{ HTTP_METHOD_GET,	"/ams/download-register-modal",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "Modal Register Form");
			}
		}},
		{ HTTP_METHOD_GET,	"/ams/download-login-modal",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "Modal Login Form");
			}
		}},
		{ HTTP_METHOD_GET,	"/ams/download-css",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "AMS stiles");
			}
		}},
		{ HTTP_METHOD_GET,	"/ams/download-js",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "AMS scripts");
			}
		}},
		{ HTTP_METHOD_GET,	"/ams/register-page",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "AMS register page");
			}
		}},
		{ HTTP_METHOD_GET,	"/ams/login-page",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "AMS login page");
			}
		}},
		//...
		{0,	NULL,	NULL}
	};

public:
	BASE(cAms, "cAms", RF_CLONE, 1,0,0);

	bool object_ctl(_u32 cmd, void *arg, ...) {
		bool r = false;

		switch(cmd) {
			case OCTL_INIT:
				m_use_doc_root = false;
				m_ams_path = NULL;
				m_db_host = NULL;
				m_db_port = "5432";
				m_db_user = NULL;
				m_db_pass = NULL;
				m_db_name = NULL;
				mpi_args = dynamic_cast<iArgs *>(_gpi_repo_->object_by_iname(I_ARGS, RF_CLONE|RF_NONOTIFY));
				mpi_log = dynamic_cast<iLog *>(_gpi_repo_->object_by_iname(I_LOG, RF_ORIGINAL));

				if(mpi_args && mpi_log)
					r = true;
				break;
			case OCTL_UNINIT:
				_gpi_repo_->object_release(mpi_args, false);
				_gpi_repo_->object_release(mpi_log);
				r = true;
				break;
		}

		return r;
	}

	bool options(_cstr_t opt) {
		bool r = mpi_args->init(opt);

		if(r) {
			m_use_doc_root = mpi_args->check("use-doc-root");
			m_ams_path = mpi_args->value("path");
			m_db_host = mpi_args->value("db-host");
			if(!(m_db_port = mpi_args->value("db-port")))
				m_db_port = "5432";
			m_db_user = mpi_args->value("db-user");
			m_db_pass = mpi_args->value("db-pass");
			if(!(m_db_name = mpi_args->value("db-name")))
				m_db_name = "ams";
		}

		return r;
	}

	bool attach(_server_t *p_srv, _cstr_t host=NULL) {
		bool r = true;
		_u32 n = 0;

		m_dbci.init(m_db_host, m_db_port,
			m_db_user, m_db_pass,
			m_db_name);

		while(m_route[n].method) {
			p_srv->on_route(m_route[n].method, m_route[n].path, m_route[n].handler, this, host);
			n++;
		}

		return r;
	}

	void detach(_server_t *p_srv, _cstr_t host=NULL) {
		_u32 n = 0;

		while(m_route[n].method) {
			p_srv->remove_route(m_route[n].method, m_route[n].path, host);
			n++;
		}

		m_dbci.destroy();
	}
};

static cAms _g_ams_;
