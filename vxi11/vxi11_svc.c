
#include "vxi11.h"
#include <memory.h>
#include <netinet/in.h>
#include <rpc/pmap_clnt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "vxi11_svc.h"
#include "../utils/scpi-def.h"
#include "../utils/utils.h"
#include "../pm/param.h"
#include "../hislip/scpi_transport.h"
#include "scpi.h"

#ifndef SIG_PF
#define SIG_PF void (*)(int)
#endif

#define RX_BUF_SIZE 512

char available_bytes = 0;

enum {
    VXI11_CORE_ERROR_NO_ERROR = 0,
    VXI11_CORE_ERROR_SYNTAX_ERROR = 1,
    VXI11_CORE_ERROR_DEVICE_NOT_ACCESSIBLE = 3,
    VXI11_CORE_ERROR_INVALID_ID = 4,
    VXI11_CORE_ERROR_PARAMETER_ERROR = 5,
    VXI11_CORE_ERROR_CHANNEL_NOT_ESTABLISHED = 6,
    VXI11_CORE_ERROR_OPERATION_NOT_SUPPORTED = 8,
    VXI11_CORE_ERROR_OUT_OF_RESOURCES = 9,
    VXI11_CORE_ERROR_DEVICE_LOCKED = 11,
    VXI11_CORE_ERROR_NO_LOCK_HELD = 12,
    VXI11_CORE_ERROR_IO_TIMEOUT = 15,
    VXI11_CORE_ERROR_IO_ERROR = 17,
    VXI11_CORE_ERROR_INVALID_ADDRESS = 21,
    VXI11_CORE_ERROR_ABORT = 23,
    VXI11_CORE_ERROR_CHANNEL_ALREADY_ESTABLISHED = 29
};

enum {
    VXI11_REASON_REQCNT = 1,
    VXI11_REASON_CHR = 2,
    VXI11_REASON_END = 4
};

bool_t
device_abort_1_svc(Device_Link* argp, Device_Error* result, struct svc_req* rqstp)
{
    bool_t retval;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

int device_async_1_freeresult(SVCXPRT* transp, xdrproc_t xdr_result, caddr_t result)
{
    xdr_free(xdr_result, result);
    return 1;
}

bool_t
create_link_1_svc(Create_LinkParms* argp, Create_LinkResp* result, struct svc_req* rqstp)
{
	if(strcmp(argp->device, "inst0") == 0){
    	result->lid = 0;
    	result->maxRecvSize = RX_BUF_SIZE;
    	result->abortPort = rqstp->rq_xprt->xp_port;
    	result->error = VXI11_CORE_ERROR_NO_ERROR;
	}else {
		result->error = VXI11_CORE_ERROR_SYNTAX_ERROR;
	}
    return 1;
}

bool_t
device_write_1_svc(Device_WriteParms* argp, Device_WriteResp* result, struct svc_req* rqstp)
{
	//printhex(argp->data.data_val, argp->data.data_len, 20);

    scpi_transport_input(argp->data.data_val, argp->data.data_len);
	
    result->size = argp->data.data_len;
	result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_read_1_svc(Device_ReadParms* argp, Device_ReadResp* result, struct svc_req* rqstp)
{
    if (scpi_transport_has_output()) {

        char* data = NULL;

		//printf("requestSize:%ld\r\n", argp->requestSize);
        uint32_t in_buffer = (uint32_t)scpi_transport_output_size();
        uint32_t to_copy = (argp->requestSize > in_buffer) ? in_buffer : argp->requestSize;

        data = (unsigned char*)malloc(to_copy);
        if (NULL == data) {
            result->error = VXI11_CORE_ERROR_IO_ERROR;
            return 1;
        }

        to_copy = (uint32_t)scpi_transport_read(data, to_copy);

        result->data.data_val = (char*)data;
        result->data.data_len = (u_int)to_copy;

        result->reason = (to_copy == argp->requestSize) ? VXI11_REASON_REQCNT : 0;

        //fprintf(stderr, "in_buffer: %d\n", in_buffer);
        //fprintf(stderr, "to_copy: %d\n", to_copy);
        if (!scpi_transport_has_output()) {
            result->reason |= VXI11_REASON_END;
        }

        result->error = VXI11_CORE_ERROR_NO_ERROR;
    } else {
		result->data.data_val = NULL;
        result->error = VXI11_CORE_ERROR_IO_TIMEOUT;
    }

    return 1;
}

bool_t
device_readstb_1_svc(Device_GenericParms* argp, Device_ReadStbResp* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->stb = (u_char)SCPI_RegGet(&scpi_context, SCPI_REG_STB);
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_trigger_1_svc(Device_GenericParms* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_clear_1_svc(Device_GenericParms* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_remote_1_svc(Device_GenericParms* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_local_1_svc(Device_GenericParms* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_lock_1_svc(Device_LockParms* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_unlock_1_svc(Device_Link* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_enable_srq_1_svc(Device_EnableSrqParms* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
device_docmd_1_svc(Device_DocmdParms* argp, Device_DocmdResp* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
destroy_link_1_svc(Device_Link* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
create_intr_chan_1_svc(Device_RemoteFunc* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

bool_t
destroy_intr_chan_1_svc(void* argp, Device_Error* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)rqstp;
    result->error = VXI11_CORE_ERROR_NO_ERROR;
    return 1;
}

int device_core_1_freeresult(SVCXPRT* transp, xdrproc_t xdr_result, caddr_t result)
{
    (void)transp;
    xdr_free(xdr_result, result);
    return 1;
}

bool_t
device_intr_srq_1_svc(Device_SrqParms* argp, void* result, struct svc_req* rqstp)
{
    (void)argp;
    (void)result;
    (void)rqstp;
    return 1;
}

int device_intr_1_freeresult(SVCXPRT* transp, xdrproc_t xdr_result, caddr_t result)
{
    (void)transp;
    xdr_free(xdr_result, result);
    return 1;
}

static void
device_async_1(struct svc_req* rqstp, register SVCXPRT* transp)
{
    union {
        Device_Link device_abort_1_arg;
    } argument;
    union {
        Device_Error device_abort_1_res;
    } result;
    bool_t retval;
    xdrproc_t _xdr_argument, _xdr_result;
    bool_t (*local)(char*, void*, struct svc_req*);

    switch (rqstp->rq_proc) {
    case NULLPROC:
        (void)svc_sendreply(transp, (xdrproc_t)xdr_void, (char*)NULL);
        return;

    case device_abort:
        _xdr_argument = (xdrproc_t)xdr_Device_Link;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_abort_1_svc;
        break;

    default:
        svcerr_noproc(transp);
        return;
    }
    memset((char*)&argument, 0, sizeof(argument));
    if (!svc_getargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
        svcerr_decode(transp);
        return;
    }
    retval = (bool_t)(*local)((char*)&argument, (void*)&result, rqstp);
    if (retval > 0 && !svc_sendreply(transp, (xdrproc_t)_xdr_result, (char*)&result)) {
        svcerr_systemerr(transp);
    }
    if (!svc_freeargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
        fprintf(stderr, "%s", "unable to free arguments");
        exit(1);
    }
    if (!device_async_1_freeresult(transp, _xdr_result, (caddr_t)&result))
        fprintf(stderr, "%s", "unable to free results");

    return;
}

static void
device_core_1(struct svc_req* rqstp, register SVCXPRT* transp)
{
    union {
        Create_LinkParms create_link_1_arg;
        Device_WriteParms device_write_1_arg;
        Device_ReadParms device_read_1_arg;
        Device_GenericParms device_readstb_1_arg;
        Device_GenericParms device_trigger_1_arg;
        Device_GenericParms device_clear_1_arg;
        Device_GenericParms device_remote_1_arg;
        Device_GenericParms device_local_1_arg;
        Device_LockParms device_lock_1_arg;
        Device_Link device_unlock_1_arg;
        Device_EnableSrqParms device_enable_srq_1_arg;
        Device_DocmdParms device_docmd_1_arg;
        Device_Link destroy_link_1_arg;
        Device_RemoteFunc create_intr_chan_1_arg;
    } argument;
    union {
        Create_LinkResp create_link_1_res;
        Device_WriteResp device_write_1_res;
        Device_ReadResp device_read_1_res;
        Device_ReadStbResp device_readstb_1_res;
        Device_Error device_trigger_1_res;
        Device_Error device_clear_1_res;
        Device_Error device_remote_1_res;
        Device_Error device_local_1_res;
        Device_Error device_lock_1_res;
        Device_Error device_unlock_1_res;
        Device_Error device_enable_srq_1_res;
        Device_DocmdResp device_docmd_1_res;
        Device_Error destroy_link_1_res;
        Device_Error create_intr_chan_1_res;
        Device_Error destroy_intr_chan_1_res;
    } result;
    bool_t retval;
    xdrproc_t _xdr_argument, _xdr_result;
    bool_t (*local)(char*, void*, struct svc_req*);

    switch (rqstp->rq_proc) {
    case NULLPROC:
        (void)svc_sendreply(transp, (xdrproc_t)xdr_void, (char*)NULL);
        return;

    case create_link:
        _xdr_argument = (xdrproc_t)xdr_Create_LinkParms;
        _xdr_result = (xdrproc_t)xdr_Create_LinkResp;
        local = (bool_t(*)(char*, void*, struct svc_req*))create_link_1_svc;
        break;

    case device_write:
        _xdr_argument = (xdrproc_t)xdr_Device_WriteParms;
        _xdr_result = (xdrproc_t)xdr_Device_WriteResp;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_write_1_svc;
        break;

    case device_read:
        _xdr_argument = (xdrproc_t)xdr_Device_ReadParms;
        _xdr_result = (xdrproc_t)xdr_Device_ReadResp;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_read_1_svc;
        break;

    case device_readstb:
        _xdr_argument = (xdrproc_t)xdr_Device_GenericParms;
        _xdr_result = (xdrproc_t)xdr_Device_ReadStbResp;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_readstb_1_svc;
        break;

    case device_trigger:
        _xdr_argument = (xdrproc_t)xdr_Device_GenericParms;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_trigger_1_svc;
        break;

    case device_clear:
        _xdr_argument = (xdrproc_t)xdr_Device_GenericParms;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_clear_1_svc;
        break;

    case device_remote:
        _xdr_argument = (xdrproc_t)xdr_Device_GenericParms;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_remote_1_svc;
        break;

    case device_local:
        _xdr_argument = (xdrproc_t)xdr_Device_GenericParms;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_local_1_svc;
        break;

    case device_lock:
        _xdr_argument = (xdrproc_t)xdr_Device_LockParms;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_lock_1_svc;
        break;

    case device_unlock:
        _xdr_argument = (xdrproc_t)xdr_Device_Link;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_unlock_1_svc;
        break;

    case device_enable_srq:
        _xdr_argument = (xdrproc_t)xdr_Device_EnableSrqParms;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_enable_srq_1_svc;
        break;

    case device_docmd:
        _xdr_argument = (xdrproc_t)xdr_Device_DocmdParms;
        _xdr_result = (xdrproc_t)xdr_Device_DocmdResp;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_docmd_1_svc;
        break;

    case destroy_link:
        _xdr_argument = (xdrproc_t)xdr_Device_Link;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))destroy_link_1_svc;
        break;

    case create_intr_chan:
        _xdr_argument = (xdrproc_t)xdr_Device_RemoteFunc;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))create_intr_chan_1_svc;
        break;

    case destroy_intr_chan:
        _xdr_argument = (xdrproc_t)xdr_void;
        _xdr_result = (xdrproc_t)xdr_Device_Error;
        local = (bool_t(*)(char*, void*, struct svc_req*))destroy_intr_chan_1_svc;
        break;

    default:
        svcerr_noproc(transp);
        return;
    }
    memset((char*)&argument, 0, sizeof(argument));
    if (!svc_getargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
        svcerr_decode(transp);
        return;
    }
    retval = (bool_t)(*local)((char*)&argument, (void*)&result, rqstp);
    if (retval > 0 && !svc_sendreply(transp, (xdrproc_t)_xdr_result, (char*)&result)) {
        svcerr_systemerr(transp);
    }
    if (!svc_freeargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
        fprintf(stderr, "%s", "unable to free arguments");
        exit(1);
    }
    if (!device_core_1_freeresult(transp, _xdr_result, (caddr_t)&result))
        fprintf(stderr, "%s", "unable to free results");

    return;
}

static void
device_intr_1(struct svc_req* rqstp, register SVCXPRT* transp)
{
    union {
        Device_SrqParms device_intr_srq_1_arg;
    } argument;
    union {
    } result;
    bool_t retval;
    xdrproc_t _xdr_argument, _xdr_result;
    bool_t (*local)(char*, void*, struct svc_req*);

    switch (rqstp->rq_proc) {
    case NULLPROC:
        (void)svc_sendreply(transp, (xdrproc_t)xdr_void, (char*)NULL);
        return;

    case device_intr_srq:
        _xdr_argument = (xdrproc_t)xdr_Device_SrqParms;
        _xdr_result = (xdrproc_t)xdr_void;
        local = (bool_t(*)(char*, void*, struct svc_req*))device_intr_srq_1_svc;
        break;

    default:
        svcerr_noproc(transp);
        return;
    }
    memset((char*)&argument, 0, sizeof(argument));
    if (!svc_getargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
        svcerr_decode(transp);
        return;
    }
    retval = (bool_t)(*local)((char*)&argument, (void*)&result, rqstp);
    if (retval > 0 && !svc_sendreply(transp, (xdrproc_t)_xdr_result, (char*)&result)) {
        svcerr_systemerr(transp);
    }
    if (!svc_freeargs(transp, (xdrproc_t)_xdr_argument, (caddr_t)&argument)) {
        fprintf(stderr, "%s", "unable to free arguments");
        exit(1);
    }
    if (!device_intr_1_freeresult(transp, _xdr_result, (caddr_t)&result))
        fprintf(stderr, "%s", "unable to free results");

    return;
}

int start_rpc_service()
{
    register SVCXPRT* transp;

    if (scpi_transport_init() != 0) {
        fprintf(stderr, "%s", "unable to initialize SCPI transport.");
        exit(1);
    }

    pmap_unset(DEVICE_ASYNC, DEVICE_ASYNC_VERSION);
    pmap_unset(DEVICE_CORE, DEVICE_CORE_VERSION);
    pmap_unset(DEVICE_INTR, DEVICE_INTR_VERSION);

    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
        fprintf(stderr, "%s", "cannot create udp service.");
        exit(1);
    }
    if (!svc_register(transp, DEVICE_ASYNC, DEVICE_ASYNC_VERSION, device_async_1, IPPROTO_UDP)) {
        fprintf(stderr, "%s", "unable to register (DEVICE_ASYNC, DEVICE_ASYNC_VERSION, udp).");
        exit(1);
    }
    if (!svc_register(transp, DEVICE_CORE, DEVICE_CORE_VERSION, device_core_1, IPPROTO_UDP)) {
        fprintf(stderr, "%s", "unable to register (DEVICE_CORE, DEVICE_CORE_VERSION, udp).");
        exit(1);
    }
    if (!svc_register(transp, DEVICE_INTR, DEVICE_INTR_VERSION, device_intr_1, IPPROTO_UDP)) {
        fprintf(stderr, "%s", "unable to register (DEVICE_INTR, DEVICE_INTR_VERSION, udp).");
        exit(1);
    }

    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (transp == NULL) {
        fprintf(stderr, "%s", "cannot create tcp service.");
        exit(1);
    }
    if (!svc_register(transp, DEVICE_ASYNC, DEVICE_ASYNC_VERSION, device_async_1, IPPROTO_TCP)) {
        fprintf(stderr, "%s", "unable to register (DEVICE_ASYNC, DEVICE_ASYNC_VERSION, tcp).");
        exit(1);
    }
    if (!svc_register(transp, DEVICE_CORE, DEVICE_CORE_VERSION, device_core_1, IPPROTO_TCP)) {
        fprintf(stderr, "%s", "unable to register (DEVICE_CORE, DEVICE_CORE_VERSION, tcp).");
        exit(1);
    }
    if (!svc_register(transp, DEVICE_INTR, DEVICE_INTR_VERSION, device_intr_1, IPPROTO_TCP)) {
        fprintf(stderr, "%s", "unable to register (DEVICE_INTR, DEVICE_INTR_VERSION, tcp).");
        exit(1);
    }

    //svc_run();//����

    return 0;
}


