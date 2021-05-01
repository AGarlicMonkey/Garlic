#include "Membrane/MessageHandler.hpp"

#include "Membrane/MembraneLog.hpp"

#include <msclr/lock.h>
#include <msclr/marshal_cppstd.h>

namespace garlic::membrane {
    // clang-format off
    generic<class MessageType> 
    void MessageHandler::bindToMessage(MessageSentHandler<MessageType> ^ handler) {
        MessageDelegateWrapper<MessageType> ^wrapper{ gcnew MessageDelegateWrapper<MessageType> };
        wrapper->handler = handler;

        delegates->Add(wrapper);
    }

    generic<class MessageType>
    void MessageHandler::sendMessage(MessageType message){
        System::Type ^const baseType{ message->GetType()->BaseType };        

        if (baseType == EditorMessage::typeid) {
            msclr::lock scopedLock{ editorLock };
            editorMessages->Add((EditorMessage^)message);
        } else if (baseType == EngineMessage::typeid) {
            engineMessages->Add((EngineMessage^)message);
        } else {
            CLOVE_LOG(LOG_CATEGORY_MEMBRANE, clove::LogLevel::Warning, "{0} called with neither an Editor or Engine message! Base type is: {1}", CLOVE_FUNCTION_NAME_PRETTY, msclr::interop::marshal_as<std::string>(baseType->Name));
        }
    }

    void MessageHandler::flushEditor() {
        //All messages are dispatched from the engine thread so we need to lock the editor thread incase it sends a message while we're flushing
        msclr::lock scopedLock{ editorLock };

        for each(EditorMessage ^message in editorMessages) {
            for each(IMessageDelegateWrapper^ wrapper in delegates){
                wrapper->tryInvoke(message);
            }
        }

        editorMessages->Clear();
    }

    void MessageHandler::flushEngine(System::Windows::Threading::Dispatcher ^editorDispatcher) {
        for each(EngineMessage ^message in engineMessages) {            
            for each(IMessageDelegateWrapper^ wrapper in delegates){
                wrapper->tryInvoke(message, editorDispatcher);
            }
        }

        engineMessages->Clear();
    }
    // clang-format on
}