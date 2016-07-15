/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Liexusong <liexusong001@gmail.com>                           |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_unid.h"
#include "shm.h"

/* If you declare any globals in php_unid.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(unid)
*/

typedef struct {
    int                datacenter_id;
    volatile int       worker_id;
    long               sequence;
    unsigned long long twepoch;
    unsigned long long last_time;
    unsigned char      worker_id_bits;
    unsigned char      datacenter_id_bits;
    unsigned char      sequence_bits;
    int                worker_id_shift;
    int                datacenter_id_shift;
    int                time_left_shift;
    int                sequence_mask;
} private_context_t;

/* True global resources - no need for thread safety here */
static int le_unid;
static volatile unsigned int *last_worker_id;
static private_context_t private_data;
/* For configure's entries */
static unsigned long long twepoch;
static int datacenter_id;


ZEND_INI_MH(unid_set_datacenter_id)
{
    if (new_value_length == 0) {
        return FAILURE;
    }

    datacenter_id = atoi(new_value);
    if (datacenter_id < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_INI_MH(unid_set_twepoch)
{
    if (new_value_length == 0) {
        return FAILURE;
    }

    sscanf(new_value, "%llu", &twepoch);
    if (twepoch <= 0ULL) {
        return FAILURE;
    }

    return SUCCESS;
}

PHP_INI_BEGIN()
    PHP_INI_ENTRY("unid.datacenter", "0", PHP_INI_ALL,
          unid_set_datacenter_id)
    PHP_INI_ENTRY("unid.twepoch", "1288834974657", PHP_INI_ALL,
          unid_set_twepoch)
PHP_INI_END()

/*
 * static functions for extension
 */

static unsigned long long
unid_real_time()
{
    struct timeval tv;
    unsigned long long rv;

    if (gettimeofday(&tv, NULL) == -1) {
        return 0ULL;
    }

    rv = (unsigned long long)tv.tv_sec * 1000ULL +
         (unsigned long long)tv.tv_usec / 1000ULL;

    return rv;
}


void
unid_private_data_init(int datacenter_id,
    unsigned long long twepoch)
{
    private_data.datacenter_id = datacenter_id;
    private_data.worker_id = 0;
    private_data.sequence = 0;
    private_data.twepoch = twepoch;
    private_data.last_time = 0ULL;

    private_data.worker_id_bits = 5;
    private_data.datacenter_id_bits = 5;
    private_data.sequence_bits = 12;

    private_data.worker_id_shift = private_data.sequence_bits;
    private_data.datacenter_id_shift = private_data.sequence_bits
                                            + private_data.worker_id_bits;
    private_data.time_left_shift = private_data.sequence_bits
                                            + private_data.worker_id_bits
                                            + private_data.datacenter_id_bits;

    private_data.sequence_mask = -1 ^ (-1 << private_data.sequence_bits);
}

/* {{{ PHP_MINIT_FUNCTION
 */

static struct shm shm;

PHP_MINIT_FUNCTION(unid)
{
    REGISTER_INI_ENTRIES();

    shm.size = sizeof(*last_worker_id);
    if (shm_alloc(&shm) == -1) {
        php_printf("Fatal error: Not enough memory for alloc share memory.\n");
        return -1;
    }

    last_worker_id = shm.addr;
    *last_worker_id = 0;

    unid_private_data_init(datacenter_id, twepoch);

    return SUCCESS;
}

/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(unid)
{
    /* uncomment this line if you have INI entries
    UNREGISTER_INI_ENTRIES();
    */

    shm_free(&shm);

    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(unid)
{
    if (!private_data.worker_id) {
        private_data.worker_id = __sync_add_and_fetch(last_worker_id, 1);
    }

    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(unid)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(unid)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "unid support", "enabled");
    php_info_print_table_end();

    /* Remove comments if you have entries in php.ini
    DISPLAY_INI_ENTRIES();
    */
}
/* }}} */

PHP_FUNCTION(unid_next_id)
{
    unsigned long long times;
    unsigned long long rv;
    int len;
    char buf[128];

    times = unid_real_time();
    if (times == 0ULL) {
        RETURN_FALSE;
    }

    if (private_data.last_time == times) { /* the same time */
        private_data.sequence = (private_data.sequence + 1)
                                & private_data.sequence_mask;
        if (private_data.sequence == 0) { /* sequence around */
            for (;;) {
                times = unid_real_time();
                if (times > private_data.last_time) {
                    break;
                }
            }
        }
    } else {
        private_data.sequence = 0;
    }

    private_data.last_time = times;

    rv = (times - private_data.twepoch) << private_data.time_left_shift;
    rv |= private_data.datacenter_id << private_data.datacenter_id_shift;
    rv |= private_data.worker_id << private_data.worker_id_shift;
    rv |= private_data.sequence;

    len = sprintf(buf, "%llu", rv);

    RETURN_STRINGL(buf, len, 1);
}


PHP_FUNCTION(unid_get_time)
{
    unsigned long long id;
    char *key;
    int len, rv;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key,
            &len TSRMLS_CC) == FAILURE)
    {
        RETURN_FALSE;
    }

    if (sscanf(key, "%llu", &id) == 0) {
        RETURN_FALSE;
    }

    id = (id >> private_data.time_left_shift) + private_data.twepoch;
    rv = id / 1000ULL;

    RETURN_LONG(rv);
}


PHP_FUNCTION(unid_get_worker_id)
{
    unsigned long long id;
    int datacenter_id, worker_id;
    char *key;
    int len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key,
            &len TSRMLS_CC) == FAILURE)
    {
        RETURN_FALSE;
    }

    if (sscanf(key, "%llu", &id) == 0) {
        RETURN_FALSE;
    }

    datacenter_id = (id >> private_data.datacenter_id_shift) & 0x1FULL;
    worker_id     = (id >> private_data.worker_id_shift) & 0x1FULL;

    RETURN_LONG(worker_id);
}

/* {{{ unid_functions[]
 *
 * Every user visible function must have an entry in unid_functions[].
 */
const zend_function_entry unid_functions[] = {
    PHP_FE(unid_next_id,          NULL)
    PHP_FE(unid_get_time,         NULL)
    PHP_FE(unid_get_worker_id,    NULL)
    PHP_FE_END    /* Must be the last line in unid_functions[] */
};
/* }}} */

/* {{{ unid_module_entry
 */
zend_module_entry unid_module_entry = {
    STANDARD_MODULE_HEADER,
    "unid",
    unid_functions,
    PHP_MINIT(unid),
    PHP_MSHUTDOWN(unid),
    PHP_RINIT(unid),
    PHP_RSHUTDOWN(unid),
    PHP_MINFO(unid),
    PHP_UNID_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_UNID
ZEND_GET_MODULE(unid)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
