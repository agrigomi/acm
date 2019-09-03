#include "startup.h"
#include "iGatn.h"
#include "iArgs.h"
#include "iLog.h"

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

	_route_handlers_t g_route[16]={
		{HTTP_METHOD_GET,	"/ams/register-modal",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			cAms *obj = (cAms *)udata;

			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK, "Modal Register Form");
			}
		}},
		{HTTP_METHOD_GET,	"/ams/login-modal",	[](_u8 evt, _request_t *req, _response_t *res, void *udata) {
			cAms *obj = (cAms *)udata;

			if(evt == ON_REQUEST) {
				//...
				res->end(HTTPRC_OK,"Modal Login Form");
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
		return mpi_args->init(opt);
	}

	bool attach(_server_t *p_srv, _cstr_t host=NULL) {
		bool r = true;
		_u32 n = 0;

		while(g_route[n].method) {
			p_srv->on_route(g_route[n].method, g_route[n].path, g_route[n].handler, this, host);
			n++;
		}

		return r;
	}

	void detach(_server_t *p_srv, _cstr_t host=NULL) {
		_u32 n = 0;

		while(g_route[n].method) {
			p_srv->remove_route(g_route[n].method, g_route[n].path, host);
			n++;
		}
	}
};

static cAms _g_ams_;
