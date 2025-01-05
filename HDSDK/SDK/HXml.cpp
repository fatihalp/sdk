

#include <HXml.h>
#include <exception>
#include <HDSDK.h>



namespace detail
{


HXmlHelper::HXmlHelper(bool declaration)
{
    if (declaration) {
        doc_.InsertFirstChild(doc_.NewDeclaration());
    }
}


HXml &HXmlHelper::FindElement(HXml &parent, const char *nodeName)
{
    // 当前自身是空节点, 是root节点
    if (parent.node_ == nullptr) {
        parent.node_ = doc_.NewElement(nodeName);
        doc_.InsertEndChild(parent.node_);
        return AddElement(parent.node_);
    }

    // 检查是否是根自身节点, 这里由于上面创建的新节点是给root节点
    // 这导致一个问题, root节点无法创建同名的子节点, 但子节点可以创建同名的孙子节点
    if (strcmp(parent.node_->Value(), nodeName) == 0) {
        return parent;
    }

    // 查找下一个节点
    tinyxml2::XMLElement *next = parent.node_->FirstChildElement(nodeName);
    if (next == nullptr) {
        next = parent.node_->InsertNewChildElement(nodeName);
    }

    return AddElement(next);
}


const HXml &HXmlHelper::FindElement(const HXml &parent, const char *nodeName) const
{
    if (parent.node_ == nullptr) {
        CAT_THROW(HXmlException(nodeName));
    }

    if (parent.node_ == doc_.RootElement()) {
        if (strcmp(parent.node_->Value(), nodeName) == 0) {
            return parent;
        }
    }

    // 查找下一个节点
    tinyxml2::XMLElement *next = parent.node_->FirstChildElement(nodeName);
    if (next == nullptr) {
        CAT_THROW(HXmlException(nodeName));
    }
    for (const auto &i : node_){
        if (i->node_ == next) {
            return *i;
        }
    }
    CAT_THROW(HXmlException(nodeName));
}


HXml &HXmlHelper::AddElement(tinyxml2::XMLElement *next)
{
    for (const auto &i : node_){
        if (i->node_ == next) {
            return *i;
        }
    }

    node_.emplace_back(std::shared_ptr<HXml>(new HXml(this, next)));
    return *node_.back();
}


}


tinyxml2::XMLElement *HXml::InitResultDoc(tinyxml2::XMLDocument &doc, const std::string &guid)
{
    tinyxml2::XMLDeclaration* declaration = doc.NewDeclaration();
    doc.InsertFirstChild(declaration);
    tinyxml2::XMLElement* root = doc.NewElement("sdk");
    root->SetAttribute("guid", guid.data());
    doc.InsertEndChild(root);
    return root;
}



HXml::HXml()
    : p_(new detail::HXmlHelper())
    , node_(nullptr)
    , type_(kRoot)
{

}


HXml::HXml(tinyxml2::XMLElement *element, bool scan)
    : HXml()
{
    if (element == nullptr) {
        return ;
    }

    p_->doc_.InsertEndChild(element->DeepClone(&p_->doc_));
    node_ = p_->doc_.RootElement();

    if (scan) {
        Scan();
    }
}


HXml::~HXml()
{
    if (p_ && (type_ == kRoot || type_ == kObj)) {
        delete p_;
    }
}


std::string HXml::Dump() const
{
    tinyxml2::XMLDocument *doc = nullptr;
    switch (type_) {
    case kRoot: {
        if (!p_) {
            return "";
        }
        doc = &p_->doc_;
    } break;
    default:
        if (node_ == nullptr) {
            return "";
        }
        doc = node_->GetDocument();
        break;
    }

    if (doc == nullptr) {
        return "";
    }

    tinyxml2::XMLPrinter xmlStr;
    doc->Print(&xmlStr);
    return xmlStr.CStr();
}


bool HXml::ContainsNodes(const tinyxml2::XMLElement *node, const char *name)
{
    if (node == nullptr) {
        return false;
    }

    return node->FirstChildElement(name) != nullptr;
}



bool HXml::ContainsAttribute(const tinyxml2::XMLElement *node, const char *name)
{
    if (node == nullptr) {
        return false;
    }
    return node->FindAttribute(name) != nullptr;
}



cat::HCatBuffer HXml::GetAttribute(const char *key, const char *defaultValue) const
{
    if (node_ == nullptr) {
        return defaultValue;
    }

    if (node_->FindAttribute(key) == nullptr) {
        return defaultValue;
    }

    return node_->Attribute(key);
}


HXml &HXml::operator[](const char *nodeName)
{
    return p_->FindElement(*this, nodeName);
}


const HXml &HXml::at(const char *nodeName) const
{
    return p_->FindElement(*this, nodeName);
}


void HXml::Scan()
{
    if (p_ == nullptr) {
        return ;
    }

    auto *root = p_->doc_.RootElement();
    Scan(root);
}


HXml &HXml::emplace_back(const cat::HCatBuffer &node, HXml &&sibling)
{
    tinyxml2::XMLElement *brother = p_->doc_.NewElement(node.ConstData());
    node_->InsertEndChild(brother);
    return (p_->AddElement(brother) = sibling);
}


HXml &HXml::push_back(const cat::HCatBuffer &node, const HXml &sibling)
{
    tinyxml2::XMLElement *brother = p_->doc_.NewElement(node.ConstData());
    node_->InsertEndChild(brother);
    return (p_->AddElement(brother) = sibling);
}


HXml &HXml::NewChild(const cat::HCatBuffer &node)
{
    tinyxml2::XMLElement *brother = p_->doc_.NewElement(node.ConstData());
    node_->InsertEndChild(brother);
    return p_->AddElement(brother);
}


HXml &HXml::operator=(const HXml &value)
{
    switch (type_) {
    case kObj:
        // 这里是对象节点,
        break;
    case kRoot: {
        // p_指针为空时说明追加对象只有属性
        if (value.p_ == nullptr) {
            break;
        }

        // root节点追加对象节点, 对象节点和root节点的node都是空, 比较检查p_指针就行
        if (value.type_ == kObj) {
            if (p_ == nullptr) {
                break;
            }

            // 检查对象节点有需要追加的数据吗
            tinyxml2::XMLElement *root = value.p_->doc_.RootElement();
            if (!root) {
                break;
            }

            // 当前根节点是空节点的话直接复制对象节点数据
            if (p_->doc_.RootElement() == nullptr) {
                value.p_->doc_.DeepCopy(&p_->doc_);
                break;
            }

            // 插入到根节点数据
            p_->doc_.RootElement()->InsertEndChild(root->DeepClone(&p_->doc_));
            break;
        }

        // 这里追加子节点 [""] = {{}}
        tinyxml2::XMLElement *root = value.p_->doc_.RootElement();
        if (root) {
            node_->InsertEndChild(root->DeepClone(&p_->doc_));
        }
    } break;
    case kChild: {
        // 子节点追加对象时需要追加对象节点的所有节点 to_xml()时
        if (value.type_ != kObj) {
            break;
        }

        // 这里追加类型转换出来的对象节点 [""] = class()
        tinyxml2::XMLElement * root = value.p_->doc_.RootElement();
        if (root) {
            node_->InsertEndChild(root->DeepClone(&p_->doc_));
        }
    } break;
    default:
        break;
    }

    // 当前无节点, 将属性保存到当前属性节点
    if (node_ == nullptr) {
        attr_.insert(attr_.end(), value.attr_.begin(), value.attr_.end());
        return *this;
    }

    // 将属性集合点的属性追加到这里
    for (const auto &i : value.attr_) {
        if (i.first.Empty()) {
            continue;
        }
        node_->SetAttribute(i.first.ConstData(), i.second.ConstData());
    }
    return *this;
}


HXml::HXml(detail::HXmlHelper *p, tinyxml2::XMLElement *node)
    : p_(p)
    , node_(node)
    , type_(kChild)
{

}


void HXml::Scan(tinyxml2::XMLElement *node)
{
    if (p_ == nullptr || node == nullptr) {
        return ;
    }

    for (auto *next = node->FirstChildElement(); next; next = next->NextSiblingElement()) {
        p_->AddElement(next);
        Scan(next);
    }
}

void HXmlIterator::operator++() const
{
    if (xml_ == nullptr || xml_->node_ == nullptr) {
        return ;
    }

    auto *p = xml_->node_->NextSiblingElement(xml_->node_->Value());
    if (p) {
        xml_ = &xml_->p_->AddElement(p);
    } else {
        xml_ = nullptr;
    }
}
