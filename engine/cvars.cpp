#include <cvars.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

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

    void DrawImguiEditor() override final;
    void EditParameter(CVarParameter* p, float textWidth);

    static CVarSystemImpl* Get();

private:
    CVarParameter* InitCVar(const char* name, const char* description);
    std::unordered_map<uint32_t, CVarParameter> _savedCVars;
	std::vector<CVarParameter*> _cachedEditParameters;

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

template<typename T>
T* PtrGetCVarCurrentByIndex(int32_t index) {
	return CVarSystemImpl::Get()->GetCVarArray<T>()->GetCurrentPtr(index);
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

int32_t* AutoCVar_Int::GetPtr()
{
	return PtrGetCVarCurrentByIndex<CVarType>(_index);
}

void AutoCVar_Int::Set(int32_t val)
{
    SetCVarCurrentByIndex<CVarType>(val, _index);
}

void AutoCVar_Int::Toggle()
{
	bool enabled = Get() != 0;

	Set(enabled ? 0 : 1);
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

double* AutoCVar_Float::GetPtr()
{
	return PtrGetCVarCurrentByIndex<CVarType>(_index);
}

float AutoCVar_Float::GetFloat()
{
	return static_cast<float>(Get());
}

float* AutoCVar_Float::GetFloatPtr()
{
	float* result = reinterpret_cast<float*>(GetPtr());
	return result;
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

void CVarSystemImpl::DrawImguiEditor()
{
    static std::string searchText = "";

    ImGui::InputText("Filter",&searchText);
    static bool showAdvanced = false;
    ImGui::Checkbox("Advanced",&showAdvanced);
    ImGui::Separator();
    _cachedEditParameters.clear();
    auto addToEditList = [&](CVarParameter* parameter)
    {
        bool hidden = ((uint32_t)parameter->_flags & (uint32_t)CVarFlags::NoEdit);
        bool isAdvanced = ((uint32_t)parameter->_flags & (uint32_t)CVarFlags::Advanced);
        
        if (!hidden)
        {
            if (!(!showAdvanced && isAdvanced) && parameter->_name.find(searchText) != std::string::npos) 
            {
                _cachedEditParameters.push_back(parameter);
            }            
        }        
    };

    for (size_t i = 0; i < GetCVarArray<int32_t>()->_lastCVar; i++)
    {
        addToEditList(GetCVarArray<int32_t>()->_cvars[i]._parameter);
    }
        for (size_t i = 0; i < GetCVarArray<double>()->_lastCVar; i++)
    {
        addToEditList(GetCVarArray<double>()->_cvars[i]._parameter);
    }
    for (size_t i = 0; i < GetCVarArray<std::string>()->_lastCVar; i++)
    {
        addToEditList(GetCVarArray<std::string>()->_cvars[i]._parameter);
    }

    if (_cachedEditParameters.size() > 10)
    {
        std::unordered_map<std::string, std::vector<CVarParameter*>> categorizedParams;
        
        //insert all the edit parameters into the hashmap by category
        for (auto &&p : _cachedEditParameters)
        {
            int dotPos = p->_name.find('.');
            std::string category = "";
            
            if(dotPos != std::string::npos)
            {
                category = p->_name.substr(0, dotPos);
            }        
            auto it = categorizedParams.find(category);
            if (it == categorizedParams.end())  
            {
                categorizedParams[category] = std::vector<CVarParameter*>();
                it = categorizedParams.find(category);
            }
            it->second.push_back(p);        
        }

        for(auto& [category, parameters] : categorizedParams)
        {
            std::sort(parameters.begin(), parameters.end(), [](CVarParameter* A, CVarParameter* B)
            {
                return A->_name < B->_name;
            });
            
            if (ImGui::BeginMenu(category.c_str()))
            {
                float maxTextWidth = 0;

                for (auto &&p : parameters)
                {
                    maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(p->_name.c_str()).x);
                }
                
                for (auto &&p : parameters)
                {
                    EditParameter(p, maxTextWidth);
                }
                ImGui::EndMenu();
            }            
        }
    }
    else
    {
        std::sort(_cachedEditParameters.begin(), _cachedEditParameters.end(), [](CVarParameter* A, CVarParameter* B)
        {
            return A->_name < B->_name;
        });

        float maxTextWidth = 0;
        for (auto &&p : _cachedEditParameters)
        {
            maxTextWidth = std::max(maxTextWidth, ImGui::CalcTextSize(p->_name.c_str()).x);
        }        

        for (auto &&p : _cachedEditParameters)
        {
            EditParameter(p, maxTextWidth);
        }
    }
}

void Label(const char* label, float textWidth)
{
    constexpr float Slack = 50;
    constexpr float EditorWidth = 100;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    const ImVec2 lineStart = ImGui::GetCursorScreenPos();
    const ImGuiStyle& style = ImGui::GetStyle();
    float fullWidth = textWidth + Slack;
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImGui::Text(label);
    ImVec2 finalPos = {startPos.x + fullWidth, startPos.y};
    ImGui::SameLine();
    ImGui::SetCursorScreenPos(finalPos);
    ImGui::SetNextItemWidth(EditorWidth);
}

void CVarSystemImpl::EditParameter(CVarParameter* p, float textWidth)
{
    const bool readonlyFlag = ((uint32_t)p->_flags & (uint32_t)CVarFlags::EditReadOnly);
    const bool checkboxFlag = ((uint32_t)p->_flags & (uint32_t)CVarFlags::EditCheckBox);
    const bool dragFlag = ((uint32_t)p->_flags & (uint32_t)CVarFlags::EditFloatDrag);

    switch (p->_type)
    {
    case CVarType::INT:
        if (readonlyFlag)
        {
            std::string displayFormat = p->_name + "= %i";
            ImGui::Text(displayFormat.c_str(), GetCVarArray<int32_t>()->GetCurrent(p->_arrayIndex));            
        }
        else
        {
            if (checkboxFlag)
            {
                bool bCheckbox = GetCVarArray<int32_t>()->GetCurrent(p->_arrayIndex) != 0;
                Label(p->_name.c_str(), textWidth);
                ImGui::PushID(p->_name.c_str());
                if (ImGui::Checkbox("", &bCheckbox))
                {
                    GetCVarArray<int32_t>()->SetCurrent(bCheckbox ? 1 : 0, p->_arrayIndex);
                }
                ImGui::PopID();
            }
            else
            {
                Label(p->_name.c_str(), textWidth);
                ImGui::PushID(p->_name.c_str());
                ImGui::InputInt("", GetCVarArray<int32_t>()->GetCurrentPtr(p->_arrayIndex));
                ImGui::PopID();
            }            
        }        
        break;
    case CVarType::FLOAT:
        if (readonlyFlag)
        {
            std::string displayFormat = p->_name + "= %f";
            ImGui::Text(displayFormat.c_str(), GetCVarArray<double>()->GetCurrent(p->_arrayIndex));
        }
        else
        {
            Label(p->_name.c_str(), textWidth);
            ImGui::PushID(p->_name.c_str());
            if (dragFlag)
            {
                ImGui::InputDouble("", GetCVarArray<double>()->GetCurrentPtr(p->_arrayIndex),0,0,"%.3f");                
            }
            else
            {
                ImGui::InputDouble("", GetCVarArray<double>()->GetCurrentPtr(p->_arrayIndex),0,0,"%.3f");                
            }
            ImGui::PopID();
        }        
        break;

    case CVarType::STRING:
        if (readonlyFlag)
        {
            std::string displayFormat = p->_name + "= %s";
            ImGui::PushID(p->_name.c_str());
            ImGui::Text(displayFormat.c_str(), GetCVarArray<std::string>()->GetCurrentPtr(p->_arrayIndex));
            ImGui::PopID();
        }
        else
        {
            Label(p->_name.c_str(), textWidth);
            ImGui::InputText("", GetCVarArray<std::string>()->GetCurrentPtr(p->_arrayIndex));
            ImGui::PopID();
        }
        break;
    default:
        break;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(p->_description.c_str());
    }    
}