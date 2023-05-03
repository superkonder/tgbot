#ifndef TGBOT_KEYBOARDBUTTON_H
#define TGBOT_KEYBOARDBUTTON_H

#include "tgbot/types/KeyboardButtonPollType.h"
#include "tgbot/types/WebAppInfo.h"

#include <memory>
#include <string>

namespace TgBot {

/**
 * @brief This object represents one button of the reply keyboard.
 * For simple text buttons String can be used instead of this object to specify text of the button.
 * Optional fields webApp, requestContact, requestLocation, and requestPoll are mutually exclusive.
 *
 * @ingroup types
 */
class KeyboardButton {

public:
    typedef std::shared_ptr<KeyboardButton> Ptr;

    /**
     * @brief Text of the button.
     * If none of the optional fields are used, it will be sent as a message when the button is pressed
     */
    std::string text;

    /**
     * @brief Optional. If True, the user's phone number will be sent as a contact when the button is pressed.
     * Available in private chats only.
     */
    bool requestContact = false;

    /**
     * @brief Optional. If True, the user's current location will be sent when the button is pressed.
     * Available in private chats only.
     */
    bool requestLocation = false;

    /**
     * @brief Optional. If specified, the user will be asked to create a poll and send it to the bot when the button is pressed.
     * Available in private chats only.
     */
    KeyboardButtonPollType::Ptr requestPoll;

    /**
     * @brief Optional. If specified, the described Web App will be launched when the button is pressed.
     * The Web App will be able to send a “web_app_data” service message. 
     * Available in private chats only.
     */
    WebAppInfo::Ptr webApp;
};
}

#endif //TGBOT_KEYBOARDBUTTON_H