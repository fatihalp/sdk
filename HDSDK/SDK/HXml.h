

#ifndef __HXML_H__
#define __HXML_H__


#include <HDSDK.h>
#include <tinyxml2.h>
#include <exception>
#include <initializer_list>
#include <HCatBuffer.h>
#include <sstream>
#include <memory>
#include <type_traits>
#include <utility>
#include <list>



template <typename _Ret, typename _Value>
_Ret DataToType(const _Value &value) {
    std::stringstream ss;
    ss << value;
    _Ret result;
    ss >> result;
    return result;
}
template <>
inline std::string DataToType(const cat::HCatBuffer &value) {
    return value.GetString();
}
template <>
inline std::string DataToType(const std::string &value) {
    return value;
}
template <>
inline std::string DataToType(const bool &value) {
    return value ? "true" : "false";
}
template <>
inline bool DataToType(const std::string &value) {
    return value == "true" ? true : value == "1" ? true : false;
}
template <>
inline bool DataToType(const cat::HCatBuffer &value) {
    return DataToType<bool>(value.GetConstString());
}


class HXml;
namespace detail
{


template <typename T>
struct static_const { static constexpr T value{}; };

template <typename T>
constexpr T static_const<T>::value;

template<unsigned N> struct priority_tag : priority_tag <N - 1> {};
template<> struct priority_tag<0> {};

struct to_xml_fn
{
    template<typename BasicXmlType, typename T,
             typename = std::void_t<decltype(to_xml(std::declval<BasicXmlType&>(), std::declval<T>()))>>
    auto operator()(BasicXmlType& j, T&& val) const
        noexcept(noexcept(to_xml(j, std::forward<T>(val))))
        -> decltype(to_xml(j, std::forward<T>(val)), void())
    {
        return call(j, std::forward<T>(val), priority_tag<1>{});
    }
private:
    template<typename BasicXmlType, typename T>
    auto call(BasicXmlType& j, T&& val, priority_tag<1>) const
    noexcept(noexcept(to_xml(j, std::forward<T>(val))))
    -> decltype(to_xml(j, std::forward<T>(val)), void())
    {
        return to_xml(j, std::forward<T>(val));
    }

    template<typename BasicXmlType, typename T>
    void call(BasicXmlType&, T&&, priority_tag<0>) const noexcept
    {
        static_assert(sizeof(BasicXmlType) == 0, "could not find to_xml() method in T's namespace");
    }
};
struct from_xml_fn
{
public:
    template<typename BasicXmlType, typename T>
    auto operator()(const BasicXmlType& j, T&& val) const
    noexcept(noexcept(from_xml(j, std::forward<T>(val))))
    -> decltype(from_xml(j, std::forward<T>(val)), void())
    {
        return call(j, std::forward<T>(val), priority_tag<1>{});
    }

private:
    template<typename BasicXmlType, typename T>
    auto call(const BasicXmlType& j, T&& val, priority_tag<1>) const
    noexcept(noexcept(from_xml(j, std::forward<T>(val))))
    -> decltype(from_xml(j, std::forward<T>(val)), void())
    {
        return from_xml(j, std::forward<T>(val));
    }
    template<typename BasicXmlType, typename T>
    void call(BasicXmlType&, T&&, priority_tag<0>) const noexcept
    {
        static_assert(sizeof(BasicXmlType) == 0, "could not find from_xml() method in T's namespace");
    }
};


class HXmlHelper
{
public:
    explicit HXmlHelper(bool declaration = true);

    HXml &FindElement(HXml &parent, const char *nodeName);
    const HXml &FindElement(const HXml &parent, const char *nodeName) const;
    HXml &AddElement(tinyxml2::XMLElement *next);

private:
    friend class ::HXml;
    tinyxml2::XMLDocument doc_;
    std::list<std::shared_ptr<HXml>> node_;
};


}


#ifdef H_LINUX
namespace
{


constexpr const auto& to_xml = detail::static_const<detail::to_xml_fn>::value;
constexpr const auto& from_xml = detail::static_const<detail::from_xml_fn>::value;


}
#endif




///< HXml的异常
class HXmlException : std::exception
{
public:
    explicit HXmlException(std::string &&e) : error(std::move(e)) {}
    virtual const char* what() const noexcept override { return error.data(); }

    const std::string error;
};


class HXmlIterator
{
public:
    explicit HXmlIterator(const HXml *xml) : xml_(xml) {}

    bool operator!=(const HXmlIterator &iterator) const { return iterator.xml_ != xml_; }
    const HXml &operator*() { return *xml_; }
    void operator++() const;


private:
    mutable const HXml *xml_;
};


class HXml
{
public:
    ///< 生成sdk返回的xml, 返回in节点
    static tinyxml2::XMLElement *InitResultDoc(tinyxml2::XMLDocument &doc, const std::string &guid);

    HXml();

    ///< 将xml数据转换
    explicit HXml(tinyxml2::XMLElement *element, bool scan = true);

    HXml(HXml &&other)
        : p_(other.p_)
        , node_(other.node_)
        , attr_(std::move(other.attr_))
        , type_(other.type_)
    {
        other.p_ = nullptr;
    }

    ///< 这个用来追加xml节点, 请注意, xml需要root节点存在, to_xml实现会导致, 如果只是向节点追加子节点, 使用ToXml
    template <typename _T, typename = typename std::enable_if<
                 !std::is_same<typename std::decay<_T>::type, const char *>::value &&
                 !std::is_same<typename std::decay<_T>::type, char *>::value &&
                 !std::is_same<typename std::decay<_T>::type, std::string>::value &&
                 !std::is_same<typename std::decay<_T>::type, cat::HCatBuffer>::value &&
                 !std::is_same<typename std::decay<_T>::type, HXml>::value, _T>::type>
    HXml(_T &&value)
        : p_(new detail::HXmlHelper())
        , node_(nullptr)
        , type_(kObj)
    {
        to_xml(*this, std::forward<_T>(value));
    }

    ///< 这个用于追加子节点
    template <typename _T, typename = typename std::enable_if<
                 !std::is_same<typename std::decay<_T>::type, const char *>::value &&
                 !std::is_same<typename std::decay<_T>::type, char *>::value &&
                 !std::is_same<typename std::decay<_T>::type, std::string>::value &&
                 !std::is_same<typename std::decay<_T>::type, cat::HCatBuffer>::value &&
                 !std::is_same<typename std::decay<_T>::type, HXml>::value, _T>::type>
    HXml &ToXml(_T &&value) {
        to_xml(*this, std::forward<_T>(value));
        return *this;
    }

    ///< 单个属性值生成 HXml
    template <typename _T>
    HXml(const char *key, _T &&value) : HXml(cat::HCatBuffer(key), std::forward<_T>(value)){}
    template <typename _T>
    HXml(const std::string &key, _T &&value) : HXml(cat::HCatBuffer(key), std::forward<_T>(value)){}
    template <typename _T, typename = typename std::enable_if<
                  !std::is_same<typename std::decay<_T>::type, const char *>::value &&
                  !std::is_same<typename std::decay<_T>::type, char *>::value &&
                  !std::is_same<typename std::decay<_T>::type, std::string>::value &&
                  !std::is_same<typename std::decay<_T>::type, cat::HCatBuffer>::value, _T>::type>
    HXml(const cat::HCatBuffer &key, const _T &value) : HXml(key, cat::HCatBuffer(DataToType<std::string>(value))) {}
    HXml(const cat::HCatBuffer &key, const cat::HCatBuffer &value)
        : p_(nullptr)
        , node_(nullptr)
        , attr_{{key, value}}
        , type_(kChild)
    {}

    ///< 生成属性列表
    HXml(std::initializer_list<HXml> &&value)
        : p_(nullptr)
        , node_(nullptr)
        , type_(kChild)
    {
        for (const auto &i : value) {
            attr_.insert(attr_.end(), i.attr_.begin(), i.attr_.end());
        }
    }

    ~HXml();

    ///< 获取xml文本
    std::string Dump() const;

    ///< 获取对应的结构数据, 需自行实现 对于的 from_xml(const HXml &xml, class &userClass)
    template <typename _T>
    bool get_to(_T &value) const {
        CAT_TRY {
            from_xml(*this, value);
        } CAT_CATCH (const HXmlException &) {
            return false;
        }

        return true;
    }

    ///< 递归检查当前节点的子节点是否存在该名 如当前节点node 一级节点 a 二级节点 b ContainsNodes("a", "b") 都存在返回true, 反之返回false
    static bool ContainsNodes(const tinyxml2::XMLElement *node) { return node != nullptr; }
    static bool ContainsNodes(const tinyxml2::XMLElement *node, const char *name);
    template <typename ..._Args>
    bool ContainsNodes(const tinyxml2::XMLElement *node, const char *nodeName, _Args &&...args) const {
        if (node == nullptr) {
            return false;
        }
        const tinyxml2::XMLElement *next = node->FirstChildElement(nodeName);
        return next != nullptr && ContainsNodes(next, std::forward<_Args>(args)...);
    }
    template <typename ..._Args>
    bool ContainsNodes(const char *nodeName, _Args &&...args) const { return ContainsNodes(node_, nodeName, std::forward<_Args>(args)...); }

    ///< 检查是否包含该属性
    static bool ContainsAttribute(const tinyxml2::XMLElement *node) { return node != nullptr; }
    static bool ContainsAttribute(const tinyxml2::XMLElement *node, const char *name);
    template <typename ..._Args>
    bool ContainsAttribute(const char *nodeName, _Args &&...args) const {
        return ContainsAttribute(node_, nodeName) && ContainsAttribute(node_, std::forward<_Args>(args)...);
    }

    ///< 获取属性
    cat::HCatBuffer GetAttribute(const char *key, const char *defaultValue = nullptr) const;
    cat::HCatBuffer GetNodeName() const { if (node_ == nullptr) { return ""; } return node_->Value(); }
    cat::HCatBuffer GetFirstChildNodeName() const { if (node_ == nullptr) { return ""; } auto node = node_->FirstChildElement(); return node ? node->Value() : ""; }

    ///< 设置文本
    HXml &SetText(const cat::HCatBuffer &text) { if (node_ == nullptr) { return *this; } node_->SetText(text.ConstData()); return *this; }
    cat::HCatBuffer GetText() const { if (node_ == nullptr) { return ""; } return node_->GetText(); }

    ///< 获取节点名
    cat::HCatBuffer GetName() const { return node_ ? node_->Name() : ""; }

    ///< 获取子节点, 如不存在则创建
    ///< 无root节点会先创建root节点, 其余节点都在root节点下面
    ///< 如存在根节点后xml["root"]  xml["root"]["sdk"] 和 xml["sdk"]是一样的
    HXml &operator[](const char *nodeName);

    ///< 获取子节点, 不存在则抛 HXmlException 异常
    const HXml &at(const char *nodeName) const;

    ///< 扫描节点, 生成所有对应的节点
    void Scan();

    ///< 设置属性
    template <typename _T>
    HXml &operator=(const std::pair<const char *, _T> &value) {
        if (value.first == nullptr) {
            return *this;
        }
        node_->SetAttribute(value.first, value.second);
        return *this;
    }


    ///< HXml追加属性
    HXml &operator=(const HXml &value);

    ///< 追加兄弟节点
    HXml &emplace_back(const cat::HCatBuffer &node, HXml &&sibling);
    HXml &push_back(const cat::HCatBuffer &node, const HXml &sibling);
    HXml &NewChild(const cat::HCatBuffer &node);

    ///< 为了支持迭代兄弟节点, 但实际不存在HXml时将会创建
    HXmlIterator begin() const { return HXmlIterator(this); }
    HXmlIterator end() const { return HXmlIterator(nullptr); }

    tinyxml2::XMLElement *GetNode() const { return node_; }

private:
    friend class detail::HXmlHelper;
    friend class HXmlIterator;

    HXml(detail::HXmlHelper *p, tinyxml2::XMLElement *node);

    ///< 递归扫描所有节点
    void Scan(tinyxml2::XMLElement *node);

private:
    detail::HXmlHelper *p_;
    tinyxml2::XMLElement *node_;
    std::list<std::pair<cat::HCatBuffer, cat::HCatBuffer>> attr_;
    enum {
        kRoot,
        kObj,
        kChild,
    } type_;
};


#define CREATE_TAG_P(_doc, _parent, _node, _tagName)    \
    _node = (_doc)->NewElement(_tagName);               \
    if (_node == nullptr) {                             \
        code = cat::HErrorCode::kMemoryFailed;          \
        break;                                          \
    } else {                                            \
        (_parent)->InsertEndChild(_node);               \
    }


#define CREATE_TAG_ATTR(_doc, _node, _tagName, _attrName, _attrValue)   \
    _node = (_doc)->NewElement(_tagName);                               \
    if (_node == nullptr) {                                             \
        code = cat::HErrorCode::kMemoryFailed;                          \
        break;                                                          \
    } else {                                                            \
        _node->SetAttribute(_attrName, _attrValue);                     \
    }


#define CREATE_TAG_ATTR_P(_doc, _parent, _node, _tagName, _attrName, _attrValue)\
    _node = (_doc)->NewElement(_tagName);                                       \
    if (_node == nullptr) {                                                     \
        code = cat::HErrorCode::kMemoryFailed;                                  \
        break;                                                                  \
    } else {                                                                    \
        _node->SetAttribute(_attrName, _attrValue);                             \
        (_parent)->InsertEndChild(_node);                                       \
    }

#define PARSE_TAG_ATTR_P(_parent, _node, _tagName, _attrName, _attrValue)   \
    _node = (_parent)->FirstChildElement(_tagName);                         \
    if (_node == nullptr) {                                                 \
        code = cat::HErrorCode::kParseXmlFailed;                            \
        break;                                                              \
    } else if (_node->FindAttribute(_attrName) == nullptr) {                \
        code = cat::HErrorCode::kParseXmlFailed;                            \
        break;                                                              \
    } else {                                                                \
        _attrValue = _node->Attribute(_attrName);                           \
    }

#define PARSE_TAG_ATTR_P_TYPE(_parent, _node, _tagName, _attrName, _attrValue, _type)   \
    _node = (_parent)->FirstChildElement(_tagName);                                     \
    if (_node == nullptr) {                                                             \
        code = cat::HErrorCode::kParseXmlFailed;                                        \
        break;                                                                          \
    } else if (_node->FindAttribute(_attrName) == nullptr) {                            \
        code = cat::HErrorCode::kParseXmlFailed;                                        \
        break;                                                                          \
    } else {                                                                            \
        _attrValue = DataToType<_type>(_node->Attribute(_attrName));                    \
    }


#define PARSE_TAG_ATTR_P_NULL(_parent, _node, _tagName, _attrName, _attrValue)  \
    _node = (_parent)->FirstChildElement(_tagName);                             \
    if (_node == nullptr) {                                                     \
        _attrValue = _type();                                                   \
    } else if (_node->FindAttribute(_attrName) == nullptr) {                    \
        code = cat::HErrorCode::kParseXmlFailed;                                \
        break;                                                                  \
    } else {                                                                    \
        _attrValue = _node->Attribute(_attrName);                               \
    }


#define PARSE_TAG_ATTR_P_NULL_TYPE(_parent, _node, _tagName, _attrName, _attrValue, _type)  \
    _node = (_parent)->FirstChildElement(_tagName);                                         \
    if (_node == nullptr) {                                                                 \
        _attrValue = _type();                                                               \
    } else if (_node->FindAttribute(_attrName) == nullptr) {                                \
        code = cat::HErrorCode::kParseXmlFailed;                                            \
        break;                                                                              \
    } else {                                                                                \
        _attrValue = DataToType<_type>(_node->Attribute(_attrName));                        \
    }


#define PARSE_TAG_ATTR(_node, _tagName, _attrName, _attrValue)              \
    if (_node->FindAttribute(_attrName) == nullptr) {                       \
        code = cat::HErrorCode::kParseXmlFailed;                            \
        break;                                                              \
    } else {                                                                \
        _attrValue = _node->Attribute(_attrName);                           \
    }

#define PARSE_TAG_ATTR_TYPE(_node, _tagName, _attrName, _attrValue, _type)  \
    if (_node->FindAttribute(_attrName) == nullptr) {                       \
        code = cat::HErrorCode::kParseXmlFailed;                            \
        break;                                                              \
    } else {                                                                \
        _attrValue = DataToType<_type>(_node->Attribute(_attrName));        \
    }

#endif // HXML_H
