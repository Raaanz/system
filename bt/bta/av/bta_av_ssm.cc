/******************************************************************************
 *
 *  Copyright 2004-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

/******************************************************************************
 *
 *  This is the stream state machine for the BTA advanced audio/video.
 *
 ******************************************************************************/
#include <string.h>

#include "bt_target.h"
#include "bta_av_co.h"
#include "bta_av_int.h"

/*****************************************************************************
 * Constants and types
 ****************************************************************************/

/* state machine states */
enum {
  BTA_AV_INIT_SST,
  BTA_AV_INCOMING_SST,
  BTA_AV_OPENING_SST,
  BTA_AV_OPEN_SST,
  BTA_AV_RCFG_SST,
  BTA_AV_CLOSING_SST
};

/* state machine action enumeration list */
enum {
  BTA_AV_DO_DISC,
  BTA_AV_CLEANUP,
  BTA_AV_FREE_SDB,
  BTA_AV_CONFIG_IND,
  BTA_AV_DISCONNECT_REQ,
  BTA_AV_SECURITY_REQ,
  BTA_AV_SECURITY_RSP,
  BTA_AV_SETCONFIG_RSP,
  BTA_AV_ST_RC_TIMER,
  BTA_AV_STR_OPENED,
  BTA_AV_SECURITY_IND,
  BTA_AV_SECURITY_CFM,
  BTA_AV_DO_CLOSE,
  BTA_AV_CONNECT_REQ,
  BTA_AV_SDP_FAILED,
  BTA_AV_DISC_RESULTS,
  BTA_AV_DISC_RES_AS_ACP,
  BTA_AV_OPEN_FAILED,
  BTA_AV_GETCAP_RESULTS,
  BTA_AV_SETCONFIG_REJ,
  BTA_AV_DISCOVER_REQ,
  BTA_AV_CONN_FAILED,
  BTA_AV_DO_START,
  BTA_AV_STR_STOPPED,
  BTA_AV_RECONFIG,
  BTA_AV_DATA_PATH,
  BTA_AV_START_OK,
  BTA_AV_START_FAILED,
  BTA_AV_STR_CLOSED,
  BTA_AV_CLR_CONG,
  BTA_AV_SUSPEND_CFM,
  BTA_AV_RCFG_STR_OK,
  BTA_AV_RCFG_FAILED,
  BTA_AV_RCFG_CONNECT,
  BTA_AV_RCFG_DISCNTD,
  BTA_AV_SUSPEND_CONT,
  BTA_AV_RCFG_CFM,
  BTA_AV_RCFG_OPEN,
  BTA_AV_SECURITY_REJ,
  BTA_AV_OPEN_RC,
  BTA_AV_CHK_2ND_START,
  BTA_AV_SAVE_CAPS,
  BTA_AV_SET_USE_RC,
  BTA_AV_CCO_CLOSE,
  BTA_AV_SWITCH_ROLE,
  BTA_AV_ROLE_RES,
  BTA_AV_DELAY_CO,
  BTA_AV_OPEN_AT_INC,
  BTA_AV_OFFLOAD_REQ,
  BTA_AV_OFFLOAD_RSP,
  BTA_AV_NUM_SACTIONS
};

#define BTA_AV_SIGNORE BTA_AV_NUM_SACTIONS

/* state table information */
/* #define BTA_AV_SACTION_COL           0       position of actions */
#define BTA_AV_SACTIONS 2    /* number of actions */
#define BTA_AV_SNEXT_STATE 2 /* position of next state */
#define BTA_AV_NUM_COLS 3    /* number of columns in state tables */

/* state table for init state */
static const uint8_t bta_av_sst_init[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1               Action 2 Next state */
    /* API_OPEN_EVT */ {BTA_AV_DO_DISC, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_CLOSE_EVT */ {BTA_AV_CLEANUP, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* AP_START_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* AP_STOP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* API_RECONFIG_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* API_PROTECT_REQ_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* API_PROTECT_RSP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* API_RC_OPEN_EVT  */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* SRC_DATA_READY_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* CI_SETCONFIG_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* CI_SETCONFIG_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* SDP_DISC_OK_EVT */ {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* SDP_DISC_FAIL_EVT */ {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_DISC_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_DISC_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_GETCAP_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_GETCAP_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_OPEN_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_OPEN_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_START_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_START_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_CLOSE_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_CONFIG_IND_EVT */
    {BTA_AV_CONFIG_IND, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_SECURITY_IND_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_SECURITY_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_WRITE_CFM_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_SUSPEND_CFM_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_RECONFIG_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* AVRC_TIMER_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* AVDT_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* AVDT_DISCONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* ROLE_CHANGE_EVT*/ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* AVDT_DELAY_RPT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* ACP_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_OFFLOAD_START_EVT */
    {BTA_AV_OFFLOAD_REQ, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* API_OFFLOAD_START_RSP_EVT */
    {BTA_AV_OFFLOAD_RSP, BTA_AV_SIGNORE, BTA_AV_INIT_SST}};

/* state table for incoming state */
static const uint8_t bta_av_sst_incoming[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1               Action 2 Next state */
    /* API_OPEN_EVT */ {BTA_AV_OPEN_AT_INC, BTA_AV_SIGNORE,
                        BTA_AV_INCOMING_SST},
    /* API_CLOSE_EVT */
    {BTA_AV_CCO_CLOSE, BTA_AV_DISCONNECT_REQ, BTA_AV_CLOSING_SST},
    /* AP_START_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* AP_STOP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_RECONFIG_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_PROTECT_REQ_EVT */
    {BTA_AV_SECURITY_REQ, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_PROTECT_RSP_EVT */
    {BTA_AV_SECURITY_RSP, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_RC_OPEN_EVT  */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* SRC_DATA_READY_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* CI_SETCONFIG_OK_EVT */
    {BTA_AV_SETCONFIG_RSP, BTA_AV_ST_RC_TIMER, BTA_AV_INCOMING_SST},
    /* CI_SETCONFIG_FAIL_EVT */
    {BTA_AV_SETCONFIG_REJ, BTA_AV_CLEANUP, BTA_AV_INIT_SST},
    /* SDP_DISC_OK_EVT */
    {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* SDP_DISC_FAIL_EVT */
    {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_DISC_OK_EVT */
    {BTA_AV_DISC_RES_AS_ACP, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_DISC_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_GETCAP_OK_EVT */
    {BTA_AV_SAVE_CAPS, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_GETCAP_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_OPEN_OK_EVT */ {BTA_AV_STR_OPENED, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_OPEN_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_START_OK_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_START_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_CLOSE_EVT */ {BTA_AV_CCO_CLOSE, BTA_AV_CLEANUP, BTA_AV_INIT_SST},
    /* STR_CONFIG_IND_EVT */
    {BTA_AV_CONFIG_IND, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_SECURITY_IND_EVT */
    {BTA_AV_SECURITY_IND, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_SECURITY_CFM_EVT */
    {BTA_AV_SECURITY_CFM, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_WRITE_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_SUSPEND_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_RECONFIG_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* AVRC_TIMER_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* AVDT_CONNECT_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* AVDT_DISCONNECT_EVT */
    {BTA_AV_CCO_CLOSE, BTA_AV_DISCONNECT_REQ, BTA_AV_CLOSING_SST},
    /* ROLE_CHANGE_EVT*/ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* AVDT_DELAY_RPT_EVT */
    {BTA_AV_DELAY_CO, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* ACP_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_OFFLOAD_START_EVT */
    {BTA_AV_OFFLOAD_REQ, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* API_OFFLOAD_START_RSP_EVT */
    {BTA_AV_OFFLOAD_RSP, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST}};

/* state table for opening state */
static const uint8_t bta_av_sst_opening[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1               Action 2 Next state */
    /* API_OPEN_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_CLOSE_EVT */ {BTA_AV_DO_CLOSE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AP_START_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* AP_STOP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_RECONFIG_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_PROTECT_REQ_EVT */
    {BTA_AV_SECURITY_REQ, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_PROTECT_RSP_EVT */
    {BTA_AV_SECURITY_RSP, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_RC_OPEN_EVT  */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* SRC_DATA_READY_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* CI_SETCONFIG_OK_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* CI_SETCONFIG_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* SDP_DISC_OK_EVT */
    {BTA_AV_CONNECT_REQ, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* SDP_DISC_FAIL_EVT */
    {BTA_AV_CONNECT_REQ, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_DISC_OK_EVT */
    {BTA_AV_DISC_RESULTS, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_DISC_FAIL_EVT */
    {BTA_AV_OPEN_FAILED, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_GETCAP_OK_EVT */
    {BTA_AV_GETCAP_RESULTS, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_GETCAP_FAIL_EVT */
    {BTA_AV_OPEN_FAILED, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_OPEN_OK_EVT */
    {BTA_AV_ST_RC_TIMER, BTA_AV_STR_OPENED, BTA_AV_OPEN_SST},
    /* STR_OPEN_FAIL_EVT */
    {BTA_AV_OPEN_FAILED, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_START_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_START_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_CLOSE_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_CONFIG_IND_EVT */
    {BTA_AV_CONFIG_IND, BTA_AV_SIGNORE, BTA_AV_INCOMING_SST},
    /* STR_SECURITY_IND_EVT */
    {BTA_AV_SECURITY_IND, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_SECURITY_CFM_EVT */
    {BTA_AV_SECURITY_CFM, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_WRITE_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_SUSPEND_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* STR_RECONFIG_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* AVRC_TIMER_EVT */
    {BTA_AV_SWITCH_ROLE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* AVDT_CONNECT_EVT */
    {BTA_AV_DISCOVER_REQ, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* AVDT_DISCONNECT_EVT */
    {BTA_AV_CONN_FAILED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* ROLE_CHANGE_EVT*/ {BTA_AV_ROLE_RES, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* AVDT_DELAY_RPT_EVT */
    {BTA_AV_DELAY_CO, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* ACP_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_OFFLOAD_START_EVT */
    {BTA_AV_OFFLOAD_REQ, BTA_AV_SIGNORE, BTA_AV_OPENING_SST},
    /* API_OFFLOAD_START_RSP_EVT */
    {BTA_AV_OFFLOAD_RSP, BTA_AV_SIGNORE, BTA_AV_OPENING_SST}};

/* state table for open state */
static const uint8_t bta_av_sst_open[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1               Action 2 Next state */
    /* API_OPEN_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* API_CLOSE_EVT */ {BTA_AV_DO_CLOSE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AP_START_EVT */ {BTA_AV_DO_START, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* AP_STOP_EVT */ {BTA_AV_STR_STOPPED, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* API_RECONFIG_EVT */ {BTA_AV_RECONFIG, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_PROTECT_REQ_EVT */
    {BTA_AV_SECURITY_REQ, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* API_PROTECT_RSP_EVT */
    {BTA_AV_SECURITY_RSP, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* API_RC_OPEN_EVT  */ {BTA_AV_SET_USE_RC, BTA_AV_OPEN_RC, BTA_AV_OPEN_SST},
    /* SRC_DATA_READY_EVT */
    {BTA_AV_DATA_PATH, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* CI_SETCONFIG_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* CI_SETCONFIG_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* SDP_DISC_OK_EVT */ {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* SDP_DISC_FAIL_EVT */ {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_DISC_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_DISC_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_GETCAP_OK_EVT */ {BTA_AV_SAVE_CAPS, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_GETCAP_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_OPEN_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_OPEN_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_START_OK_EVT */ {BTA_AV_START_OK, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_START_FAIL_EVT */
    {BTA_AV_START_FAILED, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_CLOSE_EVT */ {BTA_AV_STR_CLOSED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_CONFIG_IND_EVT */
    {BTA_AV_SETCONFIG_REJ, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_SECURITY_IND_EVT */
    {BTA_AV_SECURITY_IND, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_SECURITY_CFM_EVT */
    {BTA_AV_SECURITY_CFM, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_WRITE_CFM_EVT */
    {BTA_AV_CLR_CONG, BTA_AV_DATA_PATH, BTA_AV_OPEN_SST},
    /* STR_SUSPEND_CFM_EVT */
    {BTA_AV_SUSPEND_CFM, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_RECONFIG_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* AVRC_TIMER_EVT */
    {BTA_AV_OPEN_RC, BTA_AV_CHK_2ND_START, BTA_AV_OPEN_SST},
    /* AVDT_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* AVDT_DISCONNECT_EVT */
    {BTA_AV_STR_CLOSED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* ROLE_CHANGE_EVT*/ {BTA_AV_ROLE_RES, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* AVDT_DELAY_RPT_EVT */ {BTA_AV_DELAY_CO, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* ACP_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* API_OFFLOAD_START_EVT */
    {BTA_AV_OFFLOAD_REQ, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* API_OFFLOAD_START_RSP_EVT */
    {BTA_AV_OFFLOAD_RSP, BTA_AV_SIGNORE, BTA_AV_OPEN_SST}};

/* state table for reconfig state */
static const uint8_t bta_av_sst_rcfg[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1               Action 2 Next state */
    /* API_OPEN_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_CLOSE_EVT */
    {BTA_AV_DISCONNECT_REQ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AP_START_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* AP_STOP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_RECONFIG_EVT */ {BTA_AV_RECONFIG, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_PROTECT_REQ_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_PROTECT_RSP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_RC_OPEN_EVT  */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* SRC_DATA_READY_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* CI_SETCONFIG_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* CI_SETCONFIG_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* SDP_DISC_OK_EVT */ {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* SDP_DISC_FAIL_EVT */ {BTA_AV_FREE_SDB, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_DISC_OK_EVT */
    {BTA_AV_DISC_RESULTS, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_DISC_FAIL_EVT */
    {BTA_AV_STR_CLOSED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_GETCAP_OK_EVT */
    {BTA_AV_GETCAP_RESULTS, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_GETCAP_FAIL_EVT */
    {BTA_AV_STR_CLOSED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_OPEN_OK_EVT */ {BTA_AV_RCFG_STR_OK, BTA_AV_SIGNORE, BTA_AV_OPEN_SST},
    /* STR_OPEN_FAIL_EVT */
    {BTA_AV_RCFG_FAILED, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_START_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_START_FAIL_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_CLOSE_EVT */ {BTA_AV_RCFG_CONNECT, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_CONFIG_IND_EVT */
    {BTA_AV_SETCONFIG_REJ, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_SECURITY_IND_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_SECURITY_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_WRITE_CFM_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* STR_SUSPEND_CFM_EVT */
    {BTA_AV_SUSPEND_CFM, BTA_AV_SUSPEND_CONT, BTA_AV_RCFG_SST},
    /* STR_RECONFIG_CFM_EVT */
    {BTA_AV_RCFG_CFM, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* AVRC_TIMER_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* AVDT_CONNECT_EVT */ {BTA_AV_RCFG_OPEN, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* AVDT_DISCONNECT_EVT */
    {BTA_AV_RCFG_DISCNTD, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* ROLE_CHANGE_EVT*/ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* AVDT_DELAY_RPT_EVT */ {BTA_AV_DELAY_CO, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* ACP_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_OFFLOAD_START_EVT */
    {BTA_AV_OFFLOAD_REQ, BTA_AV_SIGNORE, BTA_AV_RCFG_SST},
    /* API_OFFLOAD_START_RSP_EVT */
    {BTA_AV_OFFLOAD_RSP, BTA_AV_SIGNORE, BTA_AV_RCFG_SST}};

/* state table for closing state */
static const uint8_t bta_av_sst_closing[][BTA_AV_NUM_COLS] = {
    /* Event                     Action 1               Action 2 Next state */
    /* API_OPEN_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_CLOSE_EVT */
    {BTA_AV_DISCONNECT_REQ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AP_START_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AP_STOP_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_RECONFIG_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_PROTECT_REQ_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_PROTECT_RSP_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_RC_OPEN_EVT  */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* SRC_DATA_READY_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* CI_SETCONFIG_OK_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* CI_SETCONFIG_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* SDP_DISC_OK_EVT */ {BTA_AV_SDP_FAILED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* SDP_DISC_FAIL_EVT */
    {BTA_AV_SDP_FAILED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* STR_DISC_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_DISC_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_GETCAP_OK_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_GETCAP_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_OPEN_OK_EVT */ {BTA_AV_DO_CLOSE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_OPEN_FAIL_EVT */
    {BTA_AV_DISCONNECT_REQ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_START_OK_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_START_FAIL_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_CLOSE_EVT */
    {BTA_AV_DISCONNECT_REQ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_CONFIG_IND_EVT */
    {BTA_AV_SETCONFIG_REJ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_SECURITY_IND_EVT */
    {BTA_AV_SECURITY_REJ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_SECURITY_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_WRITE_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_SUSPEND_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* STR_RECONFIG_CFM_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AVRC_TIMER_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AVDT_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AVDT_DISCONNECT_EVT */
    {BTA_AV_STR_CLOSED, BTA_AV_SIGNORE, BTA_AV_INIT_SST},
    /* ROLE_CHANGE_EVT*/ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* AVDT_DELAY_RPT_EVT */
    {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* ACP_CONNECT_EVT */ {BTA_AV_SIGNORE, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_OFFLOAD_START_EVT */
    {BTA_AV_OFFLOAD_REQ, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST},
    /* API_OFFLOAD_START_RSP_EVT */
    {BTA_AV_OFFLOAD_RSP, BTA_AV_SIGNORE, BTA_AV_CLOSING_SST}};

/* type for state table */
typedef const uint8_t (*tBTA_AV_SST_TBL)[BTA_AV_NUM_COLS];

/* state table */
static const tBTA_AV_SST_TBL bta_av_sst_tbl[] = {
    bta_av_sst_init, bta_av_sst_incoming, bta_av_sst_opening,
    bta_av_sst_open, bta_av_sst_rcfg,     bta_av_sst_closing};


/*******************************************************************************
 *
 * Function         bta_av_ssm_execute
 *
 * Description      Stream state machine event handling function for AV
 *
 *
 * Returns          void
 *
 ******************************************************************************/
void bta_av_ssm_execute(tBTA_AV_SCB* p_scb, uint16_t event,
                        tBTA_AV_DATA* p_data) {
  if (p_scb == NULL) {
    /* this stream is not registered */
    APPL_TRACE_EVENT("%s: AV channel not registered", __func__);
    return;
  }

  APPL_TRACE_VERBOSE(
      "%s: peer %s AV event(0x%x)=0x%x(%s) state=%d(%s) p_scb=%p", __func__,
      p_scb->PeerAddress().ToString().c_str(), p_scb->hndl, event,
      bta_av_evt_code(event), p_scb->state, bta_av_sst_code(p_scb->state),
      p_scb);

  /* look up the state table for the current state */
  tBTA_AV_SST_TBL state_table = bta_av_sst_tbl[p_scb->state];

  /* set next state */
  event -= BTA_AV_FIRST_SSM_EVT;
  p_scb->state = state_table[event][BTA_AV_SNEXT_STATE];

  APPL_TRACE_VERBOSE("%s: peer %s AV next state=%d(%s) p_scb=%p", __func__,
                     p_scb->PeerAddress().ToString().c_str(), p_scb->state,
                     bta_av_sst_code(p_scb->state), p_scb);

  /* execute action functions */
  for (int i = 0; i < BTA_AV_SACTIONS; i++) {
    uint8_t action = state_table[event][i];
    if (action != BTA_AV_SIGNORE) {
      (*p_scb->p_act_tbl[action])(p_scb, p_data);
    } else
      break;
  }
}

/*******************************************************************************
 *
 * Function         bta_av_is_scb_opening
 *
 * Description      Returns true is scb is in opening state.
 *
 *
 * Returns          true if scb is in opening state.
 *
 ******************************************************************************/
bool bta_av_is_scb_opening(tBTA_AV_SCB* p_scb) {
  bool is_opening = false;

  if (p_scb) {
    if (p_scb->state == BTA_AV_OPENING_SST) is_opening = true;
  }

  return is_opening;
}

/*******************************************************************************
 *
 * Function         bta_av_is_scb_incoming
 *
 * Description      Returns true is scb is in incoming state.
 *
 *
 * Returns          true if scb is in incoming state.
 *
 ******************************************************************************/
bool bta_av_is_scb_incoming(tBTA_AV_SCB* p_scb) {
  bool is_incoming = false;

  if (p_scb) {
    if (p_scb->state == BTA_AV_INCOMING_SST) is_incoming = true;
  }

  return is_incoming;
}

/*******************************************************************************
 *
 * Function         bta_av_set_scb_sst_init
 *
 * Description      Set SST state to INIT.
 *                  Use this function to change SST outside of state machine.
 *
 * Returns          None
 *
 ******************************************************************************/
void bta_av_set_scb_sst_init(tBTA_AV_SCB* p_scb) {
  if (p_scb == nullptr) {
    return;
  }

  uint8_t next_state = BTA_AV_INIT_SST;

  APPL_TRACE_VERBOSE(
      "%s: peer %s AV (hndl=0x%x) state=%d(%s) next state=%d(%s) p_scb=%p",
      __func__, p_scb->PeerAddress().ToString().c_str(), p_scb->hndl,
      p_scb->state, bta_av_sst_code(p_scb->state), next_state,
      bta_av_sst_code(next_state), p_scb);

  p_scb->state = next_state;
}

/*******************************************************************************
 *
 * Function         bta_av_is_scb_init
 *
 * Description      Returns true is scb is in init state.
 *
 *
 * Returns          true if scb is in incoming state.
 *
 ******************************************************************************/
bool bta_av_is_scb_init(tBTA_AV_SCB* p_scb) {
  bool is_init = false;

  if (p_scb) {
    if (p_scb->state == BTA_AV_INIT_SST) is_init = true;
  }

  return is_init;
}

/*******************************************************************************
 *
 * Function         bta_av_set_scb_sst_incoming
 *
 * Description      Set SST state to incoming.
 *                  Use this function to change SST outside of state machine.
 *
 * Returns          None
 *
 ******************************************************************************/
void bta_av_set_scb_sst_incoming(tBTA_AV_SCB* p_scb) {
  if (p_scb) {
    p_scb->state = BTA_AV_INCOMING_SST;
  }
}

/*****************************************************************************
 *  Debug Functions
 ****************************************************************************/
/*******************************************************************************
 *
 * Function         bta_av_sst_code
 *
 * Description
 *
 * Returns          char *
 *
 ******************************************************************************/
const char* bta_av_sst_code(uint8_t state) {
  switch (state) {
    case BTA_AV_INIT_SST:
      return "INIT";
    case BTA_AV_INCOMING_SST:
      return "INCOMING";
    case BTA_AV_OPENING_SST:
      return "OPENING";
    case BTA_AV_OPEN_SST:
      return "OPEN";
    case BTA_AV_RCFG_SST:
      return "RCFG";
    case BTA_AV_CLOSING_SST:
      return "CLOSING";
    default:
      return "unknown";
  }
}
