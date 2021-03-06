// Emil Hedemalm
// 2013-06-28

/*
#include "MainMenu.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Message/Message.h"
#include "StateManager.h"
#include "Audio/AudioManager.h"

void MainMenu::InputProcessor(int action, int inputDevice){
	switch(action){
        case TEST_AUDIO:
#ifdef USE_AUDIO
            std::cout<<"\nTesting playing audio.";
			AudioMan.Play(AudioType::SFX, "AeonicEngine.ogg", false, 1.0f);
#endif
            break;
		case GO_TO_EDITOR_STATE:
			std::cout<<"\nInput>>GO_TO_EDITOR_STATE";
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
			break;
		case GO_TO_AI_TEST:
			std::cout<<"\nInput>>GO_TO_AI_TEST";
		//	StateMan.QueueState(GAME_STATE_AI_TEST);
			break;
		case GO_TO_RACING_STATE:
			std::cout<<"\nInput>>GO_TO_RACING_STATE";
			StateMan.GetStateByID(GameStateID::GAME_STATE_RACING)->ProcessMessage(new Message("set players " + String::ToString(requestedPlayers)));
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_RACING));
			break;
	}
}
*/