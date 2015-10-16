#pragma once
#include <QJsonValue>
#include <QVariant>
#include <State/Value.hpp>

namespace iscore
{
namespace convert
{

template<typename To>
To value(const iscore::Value& val);

template<>
QVariant value(const iscore::Value& val);
template<>
QJsonValue value(const iscore::Value& val);

// We require the type to crrectly read back (e.g. int / float / char)
// and as an optimization, since we may need it multiple times,
// we chose to leave the caller save it however he wants. Hence the specific API.
QString textualType(const iscore::Value& val); // For JSONValue serialization
iscore::Value toValue(const QJsonValue& val, const QString& type);
}
}
