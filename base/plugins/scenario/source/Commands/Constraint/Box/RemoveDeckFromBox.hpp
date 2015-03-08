#pragma once
#include <public_interface/command/SerializableCommand.hpp>
#include <tools/ObjectPath.hpp>

class DeckModel;
namespace Scenario
{
    namespace Command
    {
        /**
         * @brief The RemoveDeckFromBox class
         *
         * Removes a deck. All the function views will be deleted.
         */
        class RemoveDeckFromBox : public iscore::SerializableCommand
        {
                ISCORE_COMMAND
            public:
                ISCORE_COMMAND_DEFAULT_CTOR(RemoveDeckFromBox, "ScenarioControl")
                RemoveDeckFromBox(ObjectPath&& deckPath);
                RemoveDeckFromBox(ObjectPath&& boxPath, id_type<DeckModel> deckId);

                virtual void undo() override;
                virtual void redo() override;
                virtual bool mergeWith(const Command* other) override;

            protected:
                virtual void serializeImpl(QDataStream&) const override;
                virtual void deserializeImpl(QDataStream&) override;

            private:
                ObjectPath m_path;
                id_type<DeckModel> m_deckId {};
                int m_position {};

                QByteArray m_serializedDeckData; // Should be done in the constructor
        };
    }
}
