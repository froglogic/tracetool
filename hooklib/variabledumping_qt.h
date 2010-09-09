/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_VARIABLEDUMPING_QT_H
#define TRACELIB_VARIABLEDUMPING_QT_H

#include "dlldefs.h"
#include "tracelib_config.h"
#include "variabledumping.h"

#include <QByteArray>
#include <QChar>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVariant>

TRACELIB_NAMESPACE_BEGIN

#define TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(T) \
template <> \
TRACELIB_EXPORT VariableValue convertVariable( T val );

TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QString)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QByteArray)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QChar)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QDate)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QDateTime)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(qlonglong)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QStringList)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(QTime)
TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION(qulonglong)

#undef TRACELIB_DECLARE_TEMPLATE_SPECIALIZATION

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_VARIABLEDUMPING_QT_H)

