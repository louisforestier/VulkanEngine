#include <cvars.h>
#include <string>
#include <unordered_map>

enum class CVarType : char
{
    INT,
    FLOAT,
    STRING
};

class CVarParameter
{
public:
    int32_t _arrayIndex;
    CVarType _type;
    CVarFlags _flags;
    std::string _name;
    std::string _description;
};

template<typename T>
struct CVarStorage
{
    T _initial;
    T _current;
    CVarParameter* _parameter;
};

template<typename T>
struct CVarArray
{
    CVarStorage<T>* _cvars;
    int32_t _lastCVar{0};

    CVarArray(size_t size)
    {
        _cvars = new CVarStorage<T>[size]();
    }

    ~CVarArray()
    {
        delete _cvars;
    }

    T GetCurrent(int32_t index)
    {
        return _cvars[index]._current;
    }
    
    T* GetCurrentPtr(int32_t index)
    {
        return &_cvars[index]._current;
    }

    void SetCurrent(const T& val, int32_t index)
    {
        _cvars[index]._current = val;
    }

    int Add(const T& value, CVarParameter* param)
    {
        int index = _lastCVar;
        _cvars[index]._current = value;
        _cvars[index]._initial = value;
        _cvars[index]._parameter = param;

        param->_arrayIndex = index;
        _lastCVar++;
        return index;
    }

    int Add(const T& defaultValue, const T& currentValue, CVarParameter* param)
    {
        int index = _lastCVar;
        _cvars[index]._current = currentValue;
        _cvars[index]._initial = defaultValue;
        _cvars[index]._parameter = param;

        param->_arrayIndex = index;
        _lastCVar++;
        return index;
    }

    CVarStorage<T>* GetCurrentStorage(int32_t index)
    {
        return &_cvars[index];
    }
};

class CVarSystemImpl : public CVarSystem
{
public:
    constexpr static int MAX_INT_CVARS = 1000;
    CVarArray<int32_t> _intCVars2{MAX_INT_CVARS};

    constexpr static int MAX_DOUBLE_CVARS = 1000;
    CVarArray<double> _floatCVars{MAX_DOUBLE_CVARS};

    constexpr static int MAX_STRING_CVARS = 200;
    CVarArray<std::string> _stringCVars{MAX_STRING_CVARS};


    //using templates with specializations to get the cvar arrays for each type.
    //if you try to use a type that doesn't have specialization, it will trigger a linked error.
    template<typename T>
    CVarArray<T>* GetCVarArray();

    CVarParameter* GetCVar(StringUtils::StringHash hash) override final;

    CVarParameter* CreateIntCVar(const char* name, const char* description, int32_t defaultValue, int32_t curentValue) override final;
    CVarParameter* CreateFloatCVar(const char* name, const char* description, double defaultValue, double currentValue) override final;
    CVarParameter* CreateStringCVar(const char* name, const char* description, const std::string& defaultValue, const std::string& curentValue) override final;

    double* GetFloatCVar(StringUtils::StringHash hash) override final;
    void SetFloatCVar(StringUtils::StringHash hash, double value) override final;
    int32_t* GetIntCVar(StringUtils::StringHash hash) override final;
    void SetIntCVar(StringUtils::StringHash hash, int32_t value) override final;
    const std::string* GetStringCVar(StringUtils::StringHash hash) override final;
    void SetStringCVar(StringUtils::StringHash hash, const std::string& value) override final;

    static CVarSystemImpl* Get();

private:
    CVarParameter* InitCVar(const char* name, const char* description);
    std::unordered_map<uint32_t, CVarParameter> _savedCVars;

    //templated get-set cvar versions for syntax sugar
    template<typename T>
    T* GetCVarCurrent(uint32_t namehash)
    {
        CVarParameter* par = GetCVar(namehash);
        if (!par)
        {
            return nullptr;
        }
        else
        {
            return GetCVarArray<T>()->GetCurrentPtr(par->_arrayIndex);
        }
    }

    template<typename T>
    void SetCVarCurrent(uint32_t namehash, const T& value)
    {
        CVarParameter* par = GetCVar(namehash);
        if (par)
        {
            GetCVarArray<T>()->SetCurrent(value, par->_arrayIndex);
        }
        
    }

};

    template<>
    CVarArray<int32_t>* CVarSystemImpl::GetCVarArray()
    {
        return &_intCVars2;
    }

    template<>
    CVarArray<double>* CVarSystemImpl::GetCVarArray()
    {
        return &_floatCVars;
    }

    template<>
    CVarArray<std::string>* CVarSystemImpl::GetCVarArray()
    {
        return &_stringCVars;
    }


CVarSystem* CVarSystem::Get()
{
    static CVarSystemImpl cvarSys{};
    return &cvarSys;
}

CVarSystemImpl* CVarSystemImpl::Get()
{
    return static_cast<CVarSystemImpl*>(CVarSystem::Get());
}


CVarParameter* CVarSystemImpl::InitCVar(const char* name, const char* description)
{
    if (GetCVar(name)) return nullptr; // return null if the cvar already exists

    uint32_t namehash = StringUtils::StringHash{name};
    _savedCVars[namehash] = CVarParameter();
    CVarParameter& newParam = _savedCVars[namehash];
    newParam._name = name;
    newParam._description = description;

    return &newParam;
}

CVarParameter* CVarSystemImpl::GetCVar(StringUtils::StringHash hash)
{
    auto it = _savedCVars.find(hash);
    if (it != _savedCVars.end())
    {
        return &(*it).second;
    }
    return nullptr;
}

CVarParameter* CVarSystemImpl::CreateIntCVar(const char* name, const char* description, int32_t defaultValue, int32_t currentValue)
{
    CVarParameter* param = InitCVar(name, description);
    if (!param) return nullptr;

    param->_type = CVarType::INT;

    GetCVarArray<int32_t>()->Add(defaultValue, currentValue, param);
    return param;
}


CVarParameter* CVarSystemImpl::CreateFloatCVar(const char* name, const char* description, double defaultValue, double currentValue)
{
    CVarParameter* param = InitCVar(name, description);
    if (!param) return nullptr;

    param->_type = CVarType::FLOAT;

    GetCVarArray<double>()->Add(defaultValue, currentValue, param);
    return param;
}

CVarParameter* CVarSystemImpl::CreateStringCVar(const char* name, const char* description, const std::string& defaultValue, const std::string& currentValue)
{
    CVarParameter* param = InitCVar(name, description);
    if (!param) return nullptr;

    param->_type = CVarType::STRING;

    GetCVarArray<std::string>()->Add(defaultValue, currentValue, param);
    return param;
}

double* CVarSystemImpl::GetFloatCVar(StringUtils::StringHash hash) 
{
    return GetCVarCurrent<double>(hash);
}

void CVarSystemImpl::SetFloatCVar(StringUtils::StringHash hash, double value)
{
    SetCVarCurrent<double>(hash, value);
}

int32_t* CVarSystemImpl::GetIntCVar(StringUtils::StringHash hash) 
{
    return GetCVarCurrent<int32_t>(hash);
}

void CVarSystemImpl::SetIntCVar(StringUtils::StringHash hash, int32_t value)
{
    SetCVarCurrent<int32_t>(hash, value);
}

const std::string* CVarSystemImpl::GetStringCVar(StringUtils::StringHash hash) 
{
    return GetCVarCurrent<std::string>(hash);
}

void CVarSystemImpl::SetStringCVar(StringUtils::StringHash hash, const std::string& value)
{
    SetCVarCurrent<std::string>(hash, value);
}

//get the cvar data purely by type and array index
template<typename T>
T GetCVarCurrentByIndex(int32_t index)
{
    return CVarSystemImpl::Get()->GetCVarArray<T>()->GetCurrent(index);
}

//set the cvar data purely by type and array index
template<typename T>
void SetCVarCurrentByIndex(const T& data, int32_t index)
{
    CVarSystemImpl::Get()->GetCVarArray<T>()->SetCurrent(data, index);
}

//cvar int constructor
AutoCVar_Int::AutoCVar_Int(const char* name, const char* description, int32_t defaultValue, CVarFlags flags /** = None*/)
{
    CVarParameter* par = CVarSystem::Get()->CreateIntCVar(name, description, defaultValue, defaultValue);
    par->_flags = flags;
    _index = par->_arrayIndex;
}

int32_t AutoCVar_Int::Get()
{
    return GetCVarCurrentByIndex<CVarType>(_index);
}

void AutoCVar_Int::Set(int32_t val)
{
    SetCVarCurrentByIndex<CVarType>(val, _index);
}

//cvar double constructor
AutoCVar_Float::AutoCVar_Float(const char* name, const char* description, double defaultValue, CVarFlags flags /** = None*/)
{
    CVarParameter* par = CVarSystem::Get()->CreateFloatCVar(name, description, defaultValue, defaultValue);
    par->_flags = flags;
    _index = par->_arrayIndex;
}

double AutoCVar_Float::Get()
{
    return GetCVarCurrentByIndex<CVarType>(_index);
}

void AutoCVar_Float::Set(double val)
{
    SetCVarCurrentByIndex<CVarType>(val, _index);
}

//cvar string constructor
AutoCVar_String::AutoCVar_String(const char* name, const char* description, const std::string& defaultValue, CVarFlags flags /** = None*/)
{
    CVarParameter* par = CVarSystem::Get()->CreateStringCVar(name, description, defaultValue, defaultValue);
    par->_flags = flags;
    _index = par->_arrayIndex;
}

const std::string& AutoCVar_String::Get()
{
    return GetCVarCurrentByIndex<CVarType>(_index);
}

void AutoCVar_String::Set(const std::string& val)
{
    SetCVarCurrentByIndex<CVarType>(val, _index);
}
