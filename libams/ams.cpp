#include "startup.h"
#include "iGatn.h"
#include "iArgs.h"
#include "iLog.h"

IMPLEMENT_BASE_ARRAY("libams", 10);

class cAms: public iGatnExtension {
private:
	iArgs	*mpi_args;
	iLog	*mpi_log;
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
		bool r = false;

		//...

		return r;
	}

	void detach(_server_t *p_srv, _cstr_t host=NULL) {
	}
};

static cAms _g_ams_;
