/*================================================================================

    csmi/src/bb/src/csmi_bb_vg_delete.c

  © Copyright IBM Corporation 2015-2017. All Rights Reserved

    This program is licensed under the terms of the Eclipse Public License
    v1.0 as published by the Eclipse Foundation and available at
    http://www.eclipse.org/legal/epl-v10.html

    U.S. Government Users Restricted Rights:  Use, duplication or disclosure
    restricted by GSA ADP Schedule Contract with IBM Corp.

================================================================================*/
/*
 * Author: Nick Buonarota
 * Email:  nbuonar@us.ibm.com
 */
 
 /*C includes*/
 #include <string.h>
/*CSM includes*/
/*Needed for structs and functions*/
#include "csmi/include/csm_api_burst_buffer.h" 
/*Needed for infrastructure*/
#include "csmi/src/common/include/csmi_api_internal.h" 
 
#include "csmi/src/common/include/csmi_common_utils.h" 
/*Needed for infrastructure logging*/
#include "csmutil/include/csmutil_logging.h"
#include "csmutil/include/timing.h"

#define API_PARAMETER_INPUT_TYPE csm_bb_vg_delete_input_t
#define API_PARAMETER_OUTPUT_TYPE csm_bb_vg_delete_output_t

const static csmi_cmd_t expected_cmd = CSM_CMD_bb_vg_delete;

void csmi_bb_vg_delete_destroy(csm_api_object *csm_obj);

int csm_bb_vg_delete(
    csm_api_object **csm_obj, 
    API_PARAMETER_INPUT_TYPE* input, 
    API_PARAMETER_OUTPUT_TYPE** output)
{
    START_TIMING()

    // Declare variables that we will use below.
    char     *buffer               = NULL;
    uint32_t  buffer_length        = 0;
    char     *return_buffer        = NULL;
    uint32_t  return_buffer_length = 0;
    int       error_code           = CSMI_SUCCESS;
    
    // EARLY RETURN
    // Create a csm_api_object and sets its csmi cmd and the destroy function
    create_csm_api_object(csm_obj, expected_cmd, csmi_bb_vg_delete_destroy);
    
    if ( !input || 
            input->node_name == NULL || 
			input->vg_name == NULL )
    {
        if ( !input )
        {
            csmutil_logging(error, "Invalid parameter: input parameter was null." );
        }
		else if (input->node_name == NULL)
        {
            csmutil_logging(error, "Invalid parameter: 'node_name' "
                " can not be NULL." );
        }
		else if (input->vg_name == NULL)
        {
            csmutil_logging(error, "Invalid parameter: 'vg_name' "
                " can not be NULL." );
        }
        else
        {
            csmutil_logging(error, "Invalid parameter: default error" );
        }
        csm_api_object_errcode_set(*csm_obj, CSMERR_INVALID_PARAM);
        csm_api_object_errmsg_set(*csm_obj,
            strdup(csm_get_string_from_enum(csmi_cmd_err_t,CSMERR_INVALID_PARAM)));
        
        return CSMERR_INVALID_PARAM;
    }

    // EARLY RETURN
    // Construct the buffer.
    csm_serialize_struct(API_PARAMETER_INPUT_TYPE, input, &buffer, &buffer_length);
    test_serialization( csm_obj, buffer );
    
    // Send a Message to the Backend.
    error_code = csmi_sendrecv_cmd(*csm_obj, expected_cmd,
        buffer, buffer_length, &return_buffer, &return_buffer_length);
    
    // If a success was detected process it, otherwise log the failure.
    if ( error_code == CSMI_SUCCESS )
    {
        if ( csm_deserialize_struct(API_PARAMETER_OUTPUT_TYPE, output,
                return_buffer, return_buffer_length ) == 0 )
        {
            csm_api_object_set_retdata(*csm_obj, 1, *output);
            
            if ( (*output)->failure_count  > 0 )
            {
                csmutil_logging(error, "Could not delete %d volume group(s)", 
                    (*output)->failure_count );

                csm_api_object_errcode_set(*csm_obj, CSMERR_UPDATE_MISMATCH);
                csm_api_object_errmsg_set(*csm_obj,
                    strdup(csm_get_string_from_enum(csmi_cmd_err_t, CSMERR_UPDATE_MISMATCH)));
                error_code = CSMERR_UPDATE_MISMATCH;
            }
        }
        else
        {
            csmutil_logging(error, "Deserialization failed");
            csm_api_object_errcode_set(*csm_obj, CSMERR_MSG_UNPACK_ERROR);
            csm_api_object_errmsg_set(*csm_obj,
                strdup(csm_get_string_from_enum(csmi_cmd_err_t, CSMERR_MSG_UNPACK_ERROR)));
            error_code = CSMERR_MSG_UNPACK_ERROR;
        }
    }
    else
    {
        csmutil_logging(error, "csmi_sendrecv_cmd failed: %d - %s",
            error_code, csm_api_object_errmsg_get(*csm_obj));
    }
    
    // Free the buffers.
    if ( return_buffer ) free(return_buffer);
    free(buffer);
	
    END_TIMING( csmapi, trace, csm_api_object_traceid_get(*csm_obj), expected_cmd, api )

	return error_code;
}

void csmi_bb_vg_delete_destroy(csm_api_object *csm_obj)
{
    csmi_api_internal *csmi_hdl;
    API_PARAMETER_OUTPUT_TYPE *output;

    if (csm_obj == NULL || csm_obj->hdl == NULL)
    {
        csmutil_logging(warning, "%s-%d: csm_api_object not valid", __FILE__, __LINE__);
        return;
    }

    csmi_hdl = (csmi_api_internal *) csm_obj->hdl;
    if (csmi_hdl->cmd != expected_cmd)
    {
        csmutil_logging(error, "%s-%d: Unmatched CSMI cmd\n", __FILE__, __LINE__);
        return;
    }
    
    output = (API_PARAMETER_OUTPUT_TYPE *) csmi_hdl->ret_cdata;
    csm_free_struct_ptr( API_PARAMETER_OUTPUT_TYPE, output);
}
