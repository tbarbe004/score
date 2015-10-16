#include "Value.hpp"


namespace
{

class VariantToString
{
    public:
        QString operator()(const iscore::impulse_t&) const { return {}; }
        QString operator()(int i) const { return QString::number(i); }
        QString operator()(float f) const { return QString::number(f); }
        QString operator()(bool b) const {
            static const QString tr = "true";
            static const QString f = "false";
            return b ? tr : f;
        }
        QString operator()(const QString& s) const
        {
            // TODO escape ?
            return QString("\"%1\"").arg(s);
        }

        QString operator()(const QChar& c) const
        {
            return QString("'%1'").arg(c);
        }

        QString operator()(const iscore::tuple_t& t) const
        {
            QString s{"["};

            for(const auto& elt : t)
            {
                s += eggs::variants::apply(*this, elt.impl());
                s += ", ";
            }

            s+= "]";
            return s;
        }
};
}

namespace iscore {
QDataStream& operator<<(QDataStream& s, const iscore::Value& m)
{
    s << m.val;
    return s;
}

QDataStream& operator>>(QDataStream& s, iscore::Value& m)
{
    s >> m.val;
    return s;
}
}

iscore::Value iscore::Value::fromVariant(const iscore::Value::value_type& var)
{
    return iscore::Value{var};
}

bool iscore::Value::operator==(const iscore::Value& m) const
{
    return val == m.val;
}

bool iscore::Value::operator!=(const iscore::Value& m) const
{
    return val != m.val;
}

bool iscore::Value::operator<(const iscore::Value& m) const
{
    return false;
}

iscore::Value iscore::Value::fromQVariant(const QVariant& var)
{
    ISCORE_TODO;
    return {};
}

QVariant iscore::Value::toQVariant() const
{
    ISCORE_TODO;
    return {};
}

QString iscore::Value::toString() const
{
    return eggs::variants::apply(VariantToString{}, val.impl());
}


iscore::ValueImpl::ValueImpl(iscore::impulse_t v): m_variant{v} { }


iscore::ValueImpl::ValueImpl(int v): m_variant{v} { }


iscore::ValueImpl::ValueImpl(float v): m_variant{v} { }


iscore::ValueImpl::ValueImpl(double v): m_variant{(float)v} { }


iscore::ValueImpl::ValueImpl(bool v): m_variant{v} { }


iscore::ValueImpl::ValueImpl(const QString& v): m_variant{v} { }


iscore::ValueImpl::ValueImpl(QChar v): m_variant{v} { }


iscore::ValueImpl::ValueImpl(const iscore::tuple_t& v): m_variant{v} { }


iscore::ValueImpl& iscore::ValueImpl::operator=(iscore::impulse_t v) { m_variant = v; return *this; }


iscore::ValueImpl& iscore::ValueImpl::operator=(int v) { m_variant = v; return *this;  }


iscore::ValueImpl& iscore::ValueImpl::operator=(float v) { m_variant = v; return *this;  }


iscore::ValueImpl& iscore::ValueImpl::operator=(double v) { m_variant = (float)v; return *this;  }


iscore::ValueImpl& iscore::ValueImpl::operator=(bool v) { m_variant = v; return *this;  }


iscore::ValueImpl& iscore::ValueImpl::operator=(const QString& v) { m_variant = v; return *this;  }


iscore::ValueImpl& iscore::ValueImpl::operator=(QChar v) { m_variant = v; return *this;  }


iscore::ValueImpl& iscore::ValueImpl::operator=(const iscore::tuple_t& v) { m_variant = v; return *this;  }



const char*iscore::ValueImpl::typeName() const { return ""; }


int iscore::ValueImpl::type() const { return -1; }


bool iscore::ValueImpl::operator ==(const iscore::ValueImpl& other) const { return true; }


bool iscore::ValueImpl::operator !=(const iscore::ValueImpl& other) const { return true; }


bool iscore::ValueImpl::isValid() const { return {}; }


int iscore::ValueImpl::toInt() const { return {}; }


float iscore::ValueImpl::toFloat() const { return {}; }
double iscore::ValueImpl::toDouble() const { return {}; }

float iscore::ValueImpl::toFloat(bool*) const
{
    return {};
}


bool iscore::ValueImpl::toBool() const { return {}; }


QString iscore::ValueImpl::toString() const { return {}; }


QStringList iscore::ValueImpl::toStringList() const { return {}; }


QChar iscore::ValueImpl::toChar() const { return {}; }


iscore::tuple_t iscore::ValueImpl::toTuple() const { return {}; }

namespace iscore {
QDebug& operator<<(QDebug& s, const iscore::ValueImpl& m)
{
    return s;
}


QDataStream& operator<<(QDataStream& s, const iscore::ValueImpl& m)
{
    return s;
}


QDataStream& operator>>(QDataStream& s, iscore::ValueImpl& m)
{
    return s;
}
}
