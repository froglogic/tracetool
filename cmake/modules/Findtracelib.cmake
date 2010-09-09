# Dummy Find-Module to support building the example apps inside the tracelib
# sources as well as outside of it.

SET(TRACELIB_INCLUDE_DIR ${tracelib_SOURCE_DIR}/hooklib)
SET(TRACELIB_LIBRARIES tracelib)
SET(TRACELIB_QTSUPPORT_LIBRARIES tracelib_qtsupport)
