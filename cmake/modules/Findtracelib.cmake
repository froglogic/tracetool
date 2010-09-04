# Dummy Find-Module to support building the example apps inside the tracelib
# sources as well as outside of it.

SET(TRACELIB_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/core)
SET(TRACELIB_LIBRARIES tracelib)
SET(TRACELIB_QTSUPPORT_LIBRARIES tracelib_qtsupport)
