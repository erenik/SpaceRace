// Emil Hedemalm
// 2013-06-28

/*
#include "RacingState.h"
#include "Actions.h"
// Don't include all managers. Ever.

/// Creates bindings that are used for debugging purposes only
void Racing::CreateDefaultBindings(){

	std::cout<<"\nRacing::CreateDefaultBindings() called";

	inputMapping.CreateBinding("SetCameraDefault", KEY::F1);
	inputMapping.CreateBinding("SetCameraRace", KEY::F2);
	inputMapping.CreateBinding("SetCameraRear", KEY::F3);
	inputMapping.CreateBinding("SetCameraRandom", KEY::F4);
	
	/// Nullify the editorSelection.Size()
	editorSelection.Clear();

/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	/// Get pointer to this mapping
	InputMapping * mapping = &inputMapping;
	/// Create default bindings

	/// C = Create, L = List
/// Entity Physics
	mapping->CreateBinding(TOGGLE_PHYSICS_SHAPES, KEY::CTRL, KEY::P, KEY::S, "CTRL+P+S : Toggle Physics Shapes");
	mapping->CreateBinding(PAUSE_SIMULATIONS, KEY::PAUSE_BREAK, "PAUSE_BREAK : Pause simulations");

	mapping->CreateBinding(TOGGLE_MENU, KEY::ESC, "ESC : Toggle menu");

	/// First input mapping
	Binding::defaultInputDevice = InputDevice::KEYBOARD_1;
	mapping->CreateBinding(BEGIN_ACCELERATION, KEY::W)->stopAction = STOP_ACCELERATION;
	mapping->CreateBinding(BEGIN_BREAKING, KEY::S)->stopAction = STOP_BREAKING;
	mapping->CreateBinding(BEGIN_TURNING_RIGHT, KEY::D)->stopAction = STOP_TURNING_RIGHT;
	mapping->CreateBinding(BEGIN_TURNING_LEFT, KEY::A)->stopAction = STOP_TURNING_LEFT;
	mapping->CreateBinding(BEGIN_BOOST, KEY::SPACE)->stopAction = STOP_BOOST;
	mapping->CreateBinding(RESET_POSITION, KEY::R);
	mapping->CreateBinding(TOGGLE_AUTO_PILOT, KEY::CTRL, KEY::SHIFT, KEY::E);
	mapping->CreateBinding(CHANGE_CAMERA, KEY::C);

	/// Second input mapping
	Binding::defaultInputDevice = InputDevice::KEYBOARD_2;
	mapping->CreateBinding(BEGIN_ACCELERATION, KEY::UP)->stopAction = STOP_ACCELERATION;
	mapping->CreateBinding(BEGIN_BREAKING, KEY::DOWN)->stopAction = STOP_BREAKING;
	mapping->CreateBinding(BEGIN_TURNING_RIGHT, KEY::RIGHT)->stopAction = STOP_TURNING_RIGHT;
	mapping->CreateBinding(BEGIN_TURNING_LEFT, KEY::LEFT)->stopAction = STOP_TURNING_LEFT;
	mapping->CreateBinding(BEGIN_BOOST, KEY::CTRL)->stopAction = STOP_BOOST;
	mapping->CreateBinding(RESET_POSITION, KEY::HOME);
	mapping->CreateBinding(TOGGLE_AUTO_PILOT, KEY::CTRL, KEY::SHIFT, KEY::BACKSPACE);

	/// Reset it again...important!
	Binding::defaultInputDevice = InputDevice::KEYBOARD_1;

	/// Menu navigation, yush!
//	mapping->CreateBinding(PREVIOUS_UI_ELEMENT, KEY::SHIFT, KEY::TAB);
//	mapping->CreateBinding(NEXT_UI_ELEMENT, KEY::TAB);
//	mapping->CreateBinding(ACTIVATE_UI_ELEMENT, KEY::ENTER);

	mapping->CreateBinding(SHOW_RESULTS, KEY::TAB)->stopAction = HIDE_RESULTS;
	mapping->CreateBinding(SHOW_RESULTS, KEY::L)->stopAction = HIDE_RESULTS;

//	mapping->SetBlockingKeys(mapping->CreateBinding(OPEN_CONSOLE, KEY::ENTER, "ENTER : Open Console"), KEY::ALT);

//	mapping->CreateBinding(GO_TO_MAIN_MENU, KEY::CTRL, KEY::M, "CTRL+M : Go to menu");

	mapping->SetBlockingKeys(mapping->CreateBinding(SAVE_MAP_PROMPT, KEY::CTRL, KEY::S, KEY::M, "CTRL+S+M : Save map"), KEY::R);
	mapping->CreateBinding(LOAD_MAP_PROMPT, KEY::CTRL, KEY::L, "CTRL+L : Load map");
	mapping->CreateBinding(LOAD_MODEL, KEY::ALT, KEY::L, KEY::M, "ALT+L+M : Load model");
	/// List functions
	mapping->CreateBinding(LIST_MODELS, KEY::L, KEY::M, "L+M : List models");
	mapping->CreateBinding(LIST_TEXTURES, KEY::L, KEY::T, "L+T : List textures");
	mapping->CreateBinding(LIST_ENTITIES, KEY::L, KEY::E, "L+E : List entities");
	mapping->CreateBinding(LIST_SELECTION, KEY::L, KEY::S, "L+S : List selection");			/// Lists currently selected entities
	mapping->CreateBinding(LIST_ACTIONS, KEY::L, KEY::A, "L+A : List Actions");
	mapping->CreateBinding(LIST_DATA, KEY::L, KEY::D, "L+D : List selected entity Data");

	/// Selecting functions
	mapping->CreateBinding(SELECT_ALL, KEY::CTRL, KEY::S, KEY::A, "CTRL+S+A : Select all entities");				/// Selects all entities
	mapping->CreateBinding(SELECT_NEXT, KEY::TAB, "TAB : Select Next ");	mapping->SetBlockingKeys(KEY::SHIFT);		mapping->Repeatable(true);
	mapping->CreateBinding(SELECT_PREVIOUS, KEY::SHIFT, KEY::TAB, "SHIFT+TAB : Select Previous ");	mapping->Repeatable(true);
	mapping->CreateBinding(SELECT_ENTITY_PROMPT, KEY::CTRL, KEY::S, KEY::E, "CTRL+S+E : Select entity");	/// Begins prompt to select target entity/entities
	mapping->CreateBinding(ADD_TO_SELECTION_PROMPT, KEY::CTRL, KEY::A, KEY::S, "CTRL+A+S : Add to selection"); /// Begins prompt to select more entities without deselecting the previous ones.
	mapping->CreateBinding(REMOVE_FROM_SELECTION_PROMPT, KEY::CTRL, KEY::R, KEY::S, "CTRL+R+S : Remove from selection");	///
	mapping->CreateBinding(CLEAR_SELECTION, KEY::ESC, "ESC : Deselect all entities");

	/// Entity Creation
	mapping->CreateBinding(CREATE_ENTITY_PROMPT, KEY::C, KEY::E, "C+E : Create entity");

	/// Entity Deletion
	mapping->CreateBinding(DELETE_ENTITY_PROMPT, KEY::DELETE_KEY, "DELETE : Delete selected entities");	/// Opens prompt to delete selected entity/entities

	/// Entity Manipulation
	mapping->CreateBinding(TRANSLATE_ENTITY_PROMPT, KEY::ALT, KEY::T, KEY::E, "ALT+T+E : Translate entity");
	mapping->CreateBinding(RESET_ENTITY_SCALE, KEY::ALT, KEY::R, KEY::S, "ALT+R+S : Reset entity scale");
	mapping->CreateBinding(SCALE_ENTITY_PROMPT, KEY::ALT, KEY::S, KEY::E, "ALT+S+E : Scale entity");
	mapping->CreateBinding(ROTATE_ENTITY_PROMPT, KEY::ALT, KEY::R, KEY::E, "ALT+R+E : Rotate entity");
	mapping->CreateBinding(SET_TEXTURE_PROMPT, KEY::ALT, KEY::S, KEY::T, "ALT+S+T : Set texture");
	mapping->CreateBinding(SET_MODEL_PROMPT, KEY::ALT, KEY::S, KEY::M, "ALT+S+M : Set model");
	mapping->CreateBinding(SET_ENTITY_NAME_PROMPT, KEY::ALT, KEY::S, KEY::N, "ALT+S+N : Set entity name");

	/// Camera
	mapping->CreateBinding(STOP, KEY::ESC, "ESC : Stop");
	/// Navigation
	mapping->SetBlockingKeys(mapping->CreateBinding(FORWARD, KEY::W, "W : Forward"), KEY::ALT, KEY::CTRL)->stopAction = FORWARD_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(BACKWARD, KEY::S, "S : Backward"), KEY::ALT, KEY::CTRL, KEY::L)->stopAction = BACKWARD_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(LEFT, KEY::A, "A : Left"), KEY::ALT, KEY::CTRL)->stopAction = LEFT_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(RIGHT, KEY::D, "D : Right"), KEY::ALT, KEY::CTRL)->stopAction = RIGHT_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(UP, KEY::E, "E : Up"), KEY::CTRL, KEY::C, KEY::ALT)->stopAction = UP_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(DOWN, KEY::Q, "Q : Down"), KEY::ALT, KEY::CTRL)->stopAction = DOWN_S;

	/// Rotation
	mapping->SetBlockingKeys(mapping->CreateBinding(TURN_LEFT, KEY::LEFT, "LEFT : Turn left"), KEY::ALT, KEY::CTRL)->stopAction = TURN_LEFT_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(TURN_RIGHT, KEY::RIGHT, "RIGHT : Turn right"), KEY::ALT, KEY::CTRL)->stopAction = TURN_RIGHT_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(TURN_UP, KEY::UP, "UP : Turn up"), KEY::ALT, KEY::CTRL)->stopAction = TURN_UP_S;
	mapping->SetBlockingKeys(mapping->CreateBinding(TURN_DOWN, KEY::DOWN, "DOWN : Turn down"), KEY::ALT, KEY::CTRL)->stopAction = TURN_DOWN_S;
	/// Camera Distance
	mapping->SetBlockingKeys(mapping->CreateBinding(COME_CLOSER, KEY::PUNCTUATION, "PUNCTUATION : Come closer"), KEY::ALT, KEY::CTRL); mapping->Repeatable(true);
	mapping->SetBlockingKeys(mapping->CreateBinding(BACK_AWAY, KEY::COMMA, "COMMA : Back away"), KEY::ALT, KEY::CTRL); mapping->Repeatable(true);
	/// Zoom
	mapping->SetBlockingKeys(mapping->CreateBinding(ZOOM_IN, KEY::PG_UP, "PG_UP : Zoom in"), KEY::ALT, KEY::CTRL); mapping->Repeatable(true);
	mapping->SetBlockingKeys(mapping->CreateBinding(ZOOM_OUT, KEY::PG_DOWN, "PG_DOWN : Zoom out"), KEY::ALT, KEY::CTRL); mapping->Repeatable(true);
	/// Adjusting speeds
	mapping->CreateBinding(INCREASE_SPEED, KEY::PLUS, "PLUS : Increase Speed"); mapping->Repeatable(true);
	mapping->CreateBinding(DECREASE_SPEED, KEY::MINUS, "MINUS : Decrease Speed"); mapping->Repeatable(true);

	/// Home- rest camera :D
	mapping->SetBlockingKeys(mapping->CreateBinding(RESET_CAMERA, KEY::HOME, "HOME : Reset camera"), KEY::ALT, KEY::CTRL);

};

// #endif
*/