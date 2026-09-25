# Clean symbols — `src/management`

DIRECTORY=src/management
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the small PS2-side Pi management protocol. R31 begins with
one read-only configuration-document GET over an independently created private
management descriptor. It owns bounded HTTP request/status/header/body
mechanics and raw document retrieval only; it does not parse Configuration,
install Input bindings, mutate Application/UI/RFB/Transport/media state, persist
settings, or choose startup/recovery policy.

The inventory below covers maintained clean-generation symbols defined directly
in this directory.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| management_append_body | function | src/management/config_get.c | config_get | file | Defines management_append_body as a current clean-source function. | mechanically reconciled current clean source |
| management_send_request | function | src/management/config_get.c | config_get | file | Defines management_send_request as a current clean-source function. | mechanically reconciled current clean source |
| management_status_is_200 | function | src/management/config_get.c | config_get | file | Defines management_status_is_200 as a current clean-source function. | mechanically reconciled current clean source |
| pstvnc_management_config_get | function | src/management/config_get.c | config_get | file | Defines pstvnc_management_config_get as a current clean-source function. | mechanically reconciled current clean source |
| PSTVNC_MANAGEMENT_CONFIG_REQUEST_MAX_BYTES | macro | src/management/config_get.c | config_get | file | Defines PSTVNC_MANAGEMENT_CONFIG_REQUEST_MAX_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_MANAGEMENT_RECEIVE_CHUNK_BYTES | macro | src/management/config_get.c | config_get | file | Defines PSTVNC_MANAGEMENT_RECEIVE_CHUNK_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| config_request | variable | src/management/config_get.c | config_get | file | Defines config_request as a current clean-source variable. | mechanically reconciled current clean source |
| byte_count | parameter | src/management/config_get.c | management_append_body | local | Defines byte_count as a current clean-source parameter. | mechanically reconciled current clean source |
| bytes | parameter | src/management/config_get.c | management_append_body | local | Defines bytes as a current clean-source parameter. | mechanically reconciled current clean source |
| candidate | parameter | src/management/config_get.c | management_append_body | local | Defines candidate as a current clean-source parameter. | mechanically reconciled current clean source |
| candidate_length | parameter | src/management/config_get.c | management_append_body | local | Defines candidate_length as a current clean-source parameter. | mechanically reconciled current clean source |
| capacity | parameter | src/management/config_get.c | management_append_body | local | Defines capacity as a current clean-source parameter. | mechanically reconciled current clean source |
| socket_fd | parameter | src/management/config_get.c | management_send_request | local | Defines socket_fd as a current clean-source parameter. | mechanically reconciled current clean source |
| request_length | variable | src/management/config_get.c | management_send_request | local | Defines request_length as a current clean-source variable. | mechanically reconciled current clean source |
| result | variable | src/management/config_get.c | management_send_request | local | Defines result as a current clean-source variable. | mechanically reconciled current clean source |
| sent | variable | src/management/config_get.c | management_send_request | local | Defines sent as a current clean-source variable. | mechanically reconciled current clean source |
| header | parameter | src/management/config_get.c | management_status_is_200 | local | Defines header as a current clean-source parameter. | mechanically reconciled current clean source |
| header_length | parameter | src/management/config_get.c | management_status_is_200 | local | Defines header_length as a current clean-source parameter. | mechanically reconciled current clean source |
| line_end | variable | src/management/config_get.c | management_status_is_200 | local | Defines line_end as a current clean-source variable. | mechanically reconciled current clean source |
| body | parameter | src/management/config_get.c | pstvnc_management_config_get | local | Defines body as a current clean-source parameter. | mechanically reconciled current clean source |
| body_capacity | parameter | src/management/config_get.c | pstvnc_management_config_get | local | Defines body_capacity as a current clean-source parameter. | mechanically reconciled current clean source |
| body_length | parameter | src/management/config_get.c | pstvnc_management_config_get | local | Defines body_length as a current clean-source parameter. | mechanically reconciled current clean source |
| candidate_body | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines candidate_body as a current clean-source variable. | mechanically reconciled current clean source |
| candidate_length | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines candidate_length as a current clean-source variable. | mechanically reconciled current clean source |
| cursor | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines cursor as a current clean-source variable. | mechanically reconciled current clean source |
| header | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines header as a current clean-source variable. | mechanically reconciled current clean source |
| header_complete | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines header_complete as a current clean-source variable. | mechanically reconciled current clean source |
| header_length | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines header_length as a current clean-source variable. | mechanically reconciled current clean source |
| receive_buffer | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines receive_buffer as a current clean-source variable. | mechanically reconciled current clean source |
| received | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines received as a current clean-source variable. | mechanically reconciled current clean source |
| socket_fd | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines socket_fd as a current clean-source variable. | mechanically reconciled current clean source |
| success | variable | src/management/config_get.c | pstvnc_management_config_get | local | Defines success as a current clean-source variable. | mechanically reconciled current clean source |
| pstvnc_management_config_get | function declaration | src/management/config_get.h | config_get interface | public | Defines pstvnc_management_config_get as a current clean-source function declaration. | mechanically reconciled current clean source |
| PSTVNC_MANAGEMENT_CONFIG_GET_H | macro | src/management/config_get.h | config_get interface | public | Defines PSTVNC_MANAGEMENT_CONFIG_GET_H as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES | macro | src/management/config_get.h | config_get interface | public | Defines PSTVNC_MANAGEMENT_CONFIG_MAX_BODY_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| PSTVNC_MANAGEMENT_HTTP_MAX_HEADER_BYTES | macro | src/management/config_get.h | config_get interface | public | Defines PSTVNC_MANAGEMENT_HTTP_MAX_HEADER_BYTES as a current clean-source macro. | mechanically reconciled current clean source |
| body | prototype parameter | src/management/config_get.h | pstvnc_management_config_get | local | Defines body as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| body_capacity | prototype parameter | src/management/config_get.h | pstvnc_management_config_get | local | Defines body_capacity as a current clean-source prototype parameter. | mechanically reconciled current clean source |
| body_length | prototype parameter | src/management/config_get.h | pstvnc_management_config_get | local | Defines body_length as a current clean-source prototype parameter. | mechanically reconciled current clean source |
