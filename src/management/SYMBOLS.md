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
