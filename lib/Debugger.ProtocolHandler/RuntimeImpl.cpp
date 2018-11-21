// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "stdafx.h"
#include "RuntimeImpl.h"

#include "PropertyHelpers.h"
#include "ProtocolHandler.h"
#include "ProtocolHelpers.h"

#include <cassert>
#include <StringUtil.h>

namespace JsDebug
{
    using protocol::Array;
    using protocol::DictionaryValue;
    using protocol::FrontendChannel;
    using protocol::Maybe;
    using protocol::Response;
    using protocol::Runtime::CallArgument;
    using protocol::Runtime::ExceptionDetails;
    using protocol::Runtime::InternalPropertyDescriptor;
    using protocol::Runtime::PropertyDescriptor;
    using protocol::String;
    using protocol::StringUtil;
    using protocol::Value;

    namespace
    {
        const char c_ErrorInvalidObjectId[] = "Invalid object ID";
        const char c_ErrorNotEnabled[] = "Runtime is not enabled";
        const char c_ErrorNotImplemented[] = "Not implemented";
    }

    RuntimeImpl::RuntimeImpl(ProtocolHandler* handler, FrontendChannel* frontendChannel, Debugger* debugger)
        : m_timestamp(1)
        , m_handler(handler)
        , m_frontend(frontendChannel)
        , m_debugger(debugger)
        , m_contextId(1)
        , m_isEnabled(false)
    {
    }

    RuntimeImpl::~RuntimeImpl()
    {
    }

    void RuntimeImpl::evaluate(
        const String& /*in_expression*/,
        Maybe<String> /*in_objectGroup*/,
        Maybe<bool> /*in_includeCommandLineAPI*/,
        Maybe<bool> /*in_silent*/,
        Maybe<int> /*in_contextId*/,
        Maybe<bool> /*in_returnByValue*/,
        Maybe<bool> /*in_generatePreview*/,
        Maybe<bool> /*in_userGesture*/,
        Maybe<bool> /*in_awaitPromise*/,
        std::unique_ptr<EvaluateCallback> callback)
    {
        callback->sendFailure(Response::Error(c_ErrorNotImplemented));
    }

    void RuntimeImpl::awaitPromise(
        const String& /*in_promiseObjectId*/,
        Maybe<bool> /*in_returnByValue*/,
        Maybe<bool> /*in_generatePreview*/,
        std::unique_ptr<AwaitPromiseCallback> callback)
    {
        callback->sendFailure(Response::Error(c_ErrorNotImplemented));
    }

    void RuntimeImpl::callFunctionOn(
        const String& /*in_objectId*/,
        const String& /*in_functionDeclaration*/,
        Maybe<Array<CallArgument>> /*in_arguments*/,
        Maybe<bool> /*in_silent*/,
        Maybe<bool> /*in_returnByValue*/,
        Maybe<bool> /*in_generatePreview*/,
        Maybe<bool> /*in_userGesture*/,
        Maybe<bool> /*in_awaitPromise*/,
        std::unique_ptr<CallFunctionOnCallback> callback)
    {
        callback->sendFailure(Response::Error(c_ErrorNotImplemented));
    }

    Response RuntimeImpl::getProperties(
        const String& in_objectId,
        Maybe<bool> /*in_ownProperties*/,
        Maybe<bool> in_accessorPropertiesOnly,
        Maybe<bool> /*in_generatePreview*/,
        std::unique_ptr<Array<PropertyDescriptor>>* out_result,
        Maybe<Array<InternalPropertyDescriptor>>* out_internalProperties,
        Maybe<ExceptionDetails>* /*out_exceptionDetails*/)
    {
        if (in_accessorPropertiesOnly.fromMaybe(false))
        {
            // We don't support accessorPropertiesOnly queries, just return an empty list.
            *out_result = Array<PropertyDescriptor>::create();
            return Response::OK();
        }

        auto parsedId = ProtocolHelpers::ParseObjectId(in_objectId);

        int handle = 0;
        int ordinal = 0;
        String name;

        if (parsedId->getInteger(PropertyHelpers::Names::Handle, &handle))
        {
            DebuggerObject obj = m_debugger->GetObjectFromHandle(handle);
            *out_result = obj.GetPropertyDescriptors();
            *out_internalProperties = obj.GetInternalPropertyDescriptors();

            return Response::OK();
        }
        else if (parsedId->getInteger(PropertyHelpers::Names::Ordinal, &ordinal) &&
                 parsedId->getString(PropertyHelpers::Names::Name, &name))
        {
            DebuggerCallFrame callFrame = m_debugger->GetCallFrame(ordinal);

            if (name == PropertyHelpers::Names::Locals)
            {
                DebuggerLocalScope obj = callFrame.GetLocals();
                *out_result = obj.GetPropertyDescriptors();
                *out_internalProperties = obj.GetInternalPropertyDescriptors();

                return Response::OK();
            }
            else if (name == PropertyHelpers::Names::Globals)
            {
                DebuggerObject obj = callFrame.GetGlobals();
                *out_result = obj.GetPropertyDescriptors();
                *out_internalProperties = obj.GetInternalPropertyDescriptors();

                return Response::OK();
            }
        }

        return Response::Error(c_ErrorInvalidObjectId);
    }

    Response RuntimeImpl::releaseObject(const String& /*in_objectId*/)
    {
        return Response::Error(c_ErrorNotImplemented);
    }

    Response RuntimeImpl::releaseObjectGroup(const String& /*in_objectGroup*/)
    {
        return Response::Error(c_ErrorNotImplemented);
    }

    Response RuntimeImpl::runIfWaitingForDebugger()
    {
        if (!IsEnabled())
        {
            return Response::Error(c_ErrorNotEnabled);
        }

        m_handler->RunIfWaitingForDebugger();
        return Response::OK();
    }

    Response RuntimeImpl::enable()
    {
        if (m_isEnabled)
        {
            return Response::OK();
        }

        m_isEnabled = true;
        // TODO: Do other setup

        return Response::OK();
    }

    Response RuntimeImpl::disable()
    {
        if (!m_isEnabled)
        {
            return Response::OK();
        }

        m_isEnabled = false;
        // TODO: Do other cleanup

        return Response::OK();
    }

    Response RuntimeImpl::discardConsoleEntries()
    {
        return Response::Error(c_ErrorNotImplemented);
    }

    Response RuntimeImpl::setCustomObjectFormatterEnabled(bool /*in_enabled*/)
    {
        return Response::Error(c_ErrorNotImplemented);
    }

    Response RuntimeImpl::compileScript(
        const String& /*in_expression*/,
        const String& /*in_sourceURL*/,
        bool /*in_persistScript*/,
        Maybe<int> /*in_executionContextId*/,
        Maybe<String>* /*out_scriptId*/,
        Maybe<ExceptionDetails>* /*out_exceptionDetails*/)
    {
        return Response::Error(c_ErrorNotImplemented);
    }

    void RuntimeImpl::runScript(
        const String& /*in_scriptId*/,
        Maybe<int> /*in_executionContextId*/,
        Maybe<String> /*in_objectGroup*/,
        Maybe<bool> /*in_silent*/,
        Maybe<bool> /*in_includeCommandLineAPI*/,
        Maybe<bool> /*in_returnByValue*/,
        Maybe<bool> /*in_generatePreview*/,
        Maybe<bool> /*in_awaitPromise*/,
        std::unique_ptr<RunScriptCallback> callback)
    {
        callback->sendFailure(Response::Error(c_ErrorNotImplemented));
    }

    bool RuntimeImpl::IsEnabled()
    {
        return m_isEnabled;
    }

    JsValueRef RuntimeImpl::GetTypeString(JsValueRef object)
    {
        JsValueType type;
        if (JsGetValueType(object, &type) != JsNoError)
        {
            return nullptr;
        }

        const wchar_t* typeString = nullptr;
        switch (type)
        {
        case JsValueType::JsUndefined:
            typeString = L"undefined";
            break;
        case JsValueType::JsNull:
            typeString = L"null";
            break;
        case JsValueType::JsNumber:
            typeString = L"number";
            break;
        case JsValueType::JsString:
            typeString = L"string";
            break;
        case JsValueType::JsBoolean:
            typeString = L"boolean";
            break;
        case JsValueType::JsObject:
            typeString = L"object";
            break;
        case JsValueType::JsFunction:
            typeString = L"function";
            break;
        case JsValueType::JsError:
            typeString = L"error";
            break;
        case JsValueType::JsArray:
            typeString = L"array";
            break;
        case JsValueType::JsSymbol:
            typeString = L"symbol";
            break;
        case JsValueType::JsArrayBuffer:
            typeString = L"arraybuffer";
            break;
        case JsValueType::JsTypedArray:
        case JsValueType::JsDataView:
            typeString = L"typedarray";
            break;
        }

        if (typeString != nullptr)
        {
            JsValueRef typeStringObject = JS_INVALID_REFERENCE;
            if (JsPointerToString(typeString, wcslen(typeString), &typeStringObject) == JsNoError)
            {
                return typeStringObject;
            }
        }

        return nullptr;
    }

    bool RuntimeImpl::GetTypeStringAndValue(JsValueRef object, JsValueRef *typeString, JsValueRef *value)
    {
        assert(typeString != nullptr);
        assert(value != nullptr);

        JsValueRef type = GetTypeString(object);
        JsValueRef objectValue = object;
        if (type == nullptr)
        {
            if (JsConvertValueToString(object, &objectValue) != JsNoError
                || JsPointerToString(L"string", 6, &type) != JsNoError)
            {
                assert(false);
                return false;
            }
        }

        *typeString = type;
        *value = objectValue;
        return true;
    }

    void RuntimeImpl::consoleAPICalled(protocol::String type, JsValueRef *arguments, size_t argumentCount)
    {
        assert(argumentCount > 0);

        JsValueRef remoteObject = JS_INVALID_REFERENCE;
        if (JsCreateObject(&remoteObject) == JsNoError)
        {
            std::unique_ptr<protocol::Array<protocol::Runtime::RemoteObject>> args = 
                Array<protocol::Runtime::RemoteObject>::create();

            for (size_t i = 1; i < argumentCount; i++)
            {
                JsValueRef typeString = JS_INVALID_REFERENCE;
                JsValueRef objectValue = JS_INVALID_REFERENCE;

                if (GetTypeStringAndValue(arguments[i], &typeString, &objectValue))
                {
                    PropertyHelpers::SetProperty(remoteObject, PropertyHelpers::Names::Type, typeString);
                    PropertyHelpers::SetProperty(remoteObject, PropertyHelpers::Names::Value, objectValue);

                    args->addItem(ProtocolHelpers::WrapObject(remoteObject));
                }
            }

            // TODO : to get the correct context id and timestamp.
            m_frontend.consoleAPICalled(type, std::move(args), m_contextId, m_timestamp++);
        }
    }

}