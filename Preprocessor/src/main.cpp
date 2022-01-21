#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

static std::string
read_entire_file(std::string file_path)
{
	std::string result;

	std::ifstream ifs(file_path);
	if (ifs.is_open())
	{
		std::stringstream buffer;
		buffer << ifs.rdbuf();
		result = buffer.str();
	}
	else
	{
		printf("Failed to open file\n");
	}

	return result;
}

enum token_type_t
{
	TOKEN_UNKNOWN,

	TOKEN_OPEN_PAREN, TOKEN_CLOSE_PAREN,
	TOKEN_OPEN_BRACE, TOKEN_CLOSE_BRACE,
	TOKEN_OPEN_BRACKET, TOKEN_CLOSE_BRACKET,

	TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_SEMICOLON,
	TOKEN_COLON,
	TOKEN_ASTERIX,
	TOKEN_NAMESPACE,
	TOKEN_EQUALS,

	TOKEN_EOF
};

struct token_t
{
	token_type_t type;
	std::string text;
	int text_length;
};

struct tokenizer_t
{
	std::string text;
	int cursor = 0;
};

inline bool
IsEndOfLine(char c)
{
	bool result = ((c == '\n') ||
				  (c == '\r'));

	return result;
}

inline bool
IsWhitespace(char c)
{
	bool result = ((c == ' ') ||
				  (c == '\t') ||
				  (c == '\v') ||
				  (c == '\f') ||
				  IsEndOfLine(c));

	return result;
}

inline bool
IsAlpha(char c)
{
	bool result = (((c >= 'a') && (c <= 'z')) ||
				   ((c >= 'A') && (c <= 'Z')));

	return result;
}

inline bool
IsNumber(char c)
{
	bool result = ((c >= '0') && (c <= '9'));

	return result;
}

inline bool
TokenEquals(token_t token, std::string match)
{
	std::string At = match;
	int At_cursor = 0;

	for (int index = 0; index < token.text_length; index++, At_cursor++)
	{
		// At.at(index) == 0
		if ((At.size() == 0) ||
			(token.text.at(index) != At.at(At_cursor)))
		{
			return false;
		}
	}

	bool result = (At.substr(At_cursor).size() == 0);
	//bool Result = (At == 0);
	return result;
}

static void
ParseNumber(tokenizer_t* tokenizer)
{
}

static void
EatAllWhiteSpace(tokenizer_t* tokenizer)
{
	for (;;)
	{
		if (IsWhitespace(tokenizer->text.at(tokenizer->cursor)))
		{
			++tokenizer->cursor;
		}
		else if ((tokenizer->text.at(tokenizer->cursor) == '/') &&
				 (tokenizer->text.at(tokenizer->cursor + 1) == '/'))
		{
			tokenizer->cursor += 2;

			while (!IsEndOfLine(tokenizer->text.at(tokenizer->cursor)))
			{
				++tokenizer->cursor;
			}
		}
		else if ((tokenizer->text.at(tokenizer->cursor) == '/') &&
				 (tokenizer->text.at(tokenizer->cursor + 1) == '*'))
		{
			tokenizer->cursor += 2;

			while (!((tokenizer->text.at(tokenizer->cursor) == '*') &&
				     (tokenizer->text.at(tokenizer->cursor + 1) == '/')))
			{
				++tokenizer->cursor;
			}

			if (tokenizer->text.at(tokenizer->cursor) == '*')
			{
				tokenizer->cursor += 2;
			}
		}
		else
		{
			break;
		}
	}
}

static token_t
GetToken(tokenizer_t* tokenizer)
{
	EatAllWhiteSpace(tokenizer);

	token_t token = {};
	token.text_length = 1;
	token.text = tokenizer->text.substr(tokenizer->cursor);
	char c = tokenizer->text.at(tokenizer->cursor);
	++tokenizer->cursor;

	if (tokenizer->cursor >= tokenizer->text.size()) {
		token.type = TOKEN_EOF;
		return token;
	}

	switch (c)
	{
	case '(': { token.type = TOKEN_OPEN_PAREN; } break;
	case ')': { token.type = TOKEN_CLOSE_PAREN; } break;
	case '{': { token.type = TOKEN_OPEN_BRACE; } break;
	case '}': { token.type = TOKEN_CLOSE_BRACE; } break;
	case '[': { token.type = TOKEN_OPEN_BRACKET; } break;
	case ']': { token.type = TOKEN_CLOSE_BRACKET; } break;
	case '*': { token.type = TOKEN_ASTERIX; } break;
	case ';': { token.type = TOKEN_SEMICOLON; } break;
	case '=': { token.type = TOKEN_EQUALS; } break;
	case ':':
	{
		token.type = TOKEN_COLON;
		if (tokenizer->text.at(tokenizer->cursor) == ':')
		{
			token.type = TOKEN_NAMESPACE;
			token.text_length = 2;
			++tokenizer->cursor;
		}
	} break;
	case '"':
	{
		token.type = TOKEN_STRING;
		token.text = tokenizer->text.substr(tokenizer->cursor);

		while (tokenizer->text.at(tokenizer->cursor) != '"')
		{
			++tokenizer->cursor;
		}

		int text_length = token.text.size() - (tokenizer->text.size() - tokenizer->cursor);
		token.text_length = text_length;
		if (tokenizer->text.at(tokenizer->cursor) == '"')
		{
			++tokenizer->cursor;
		}
	} break;
	default:
	{
		if (IsAlpha(c) ||
			c == '~') // This is for deconstructors
		{
			token.type = TOKEN_IDENTIFIER;
			while (IsAlpha(tokenizer->text.at(tokenizer->cursor)) ||
				   IsNumber(tokenizer->text.at(tokenizer->cursor)) ||
				   tokenizer->text.at(tokenizer->cursor) == '_')
			{
				++tokenizer->cursor;
			}
			int text_length = token.text.size() - (tokenizer->text.size() - tokenizer->cursor);
			token.text_length = text_length;
		}
		else
		{
			token.type = TOKEN_UNKNOWN;
		}
	} break;
	}

	token.text.resize(token.text_length);

	//std::cout << (int)token.type << ": " << token.text << std::endl;

	return token;
}

static void
ConsumeToken(tokenizer_t* tokenizer)
{
	token_t consumed_token = GetToken(tokenizer);
}

static bool
PeakNextToken(tokenizer_t* tokenizer, token_type_t DesiredType)
{
	int og_cursor = tokenizer->cursor;
	token_t token = GetToken(tokenizer);
	tokenizer->cursor = og_cursor;

	bool result = (token.type == DesiredType);
	return result;
}

static token_t
PeakToken(tokenizer_t* tokenizer)
{
	int og_cursor = tokenizer->cursor;
	token_t token = GetToken(tokenizer);
	tokenizer->cursor = og_cursor;
	return token;
}

static bool
RequireToken(tokenizer_t* tokenizer, token_type_t DesiredType)
{
	token_t token = GetToken(tokenizer);
	bool result = (token.type == DesiredType);
	return result;
}

static void
ParseIntrospectionParams(tokenizer_t* tokenizer)
{
	for (;;)
	{
		token_t token = GetToken(tokenizer);
		if ((token.type == TOKEN_CLOSE_PAREN) ||
		    (token.type == TOKEN_EOF))
		{
			break;
		}
	}
}

static void
ParseFunction(tokenizer_t* tokenizer, token_t MemberTypeToken)
{
	bool parsing = true;
	while (parsing)
	{
		token_t token = PeakToken(tokenizer);

		switch (token.type)
		{
			case TOKEN_OPEN_PAREN:
			{
				// parse arguments
			} break;
			case TOKEN_CLOSE_BRACE:
			case TOKEN_SEMICOLON:
			case TOKEN_EOF:
			{
				parsing = false;
			} break;
		}

		if (parsing)
		{
			ConsumeToken(tokenizer);
		}
	}
}

static void
ParseMember(tokenizer_t* tokenizer, token_t StructTypeToken, token_t MemberTypeToken)
{
	bool parsing = true;
	bool is_pointer = false;
	bool is_assigment = false;
	while (parsing)
	{
		token_t token = GetToken(tokenizer);

		switch (token.type)
		{
		case TOKEN_ASTERIX:
			is_pointer = true;
			break;
		case TOKEN_NAMESPACE:
			token = GetToken(tokenizer);
			break;
		case TOKEN_IDENTIFIER:
			if (!is_assigment)
			{
				if (PeakNextToken(tokenizer, TOKEN_OPEN_PAREN))
				{
					ParseFunction(tokenizer, MemberTypeToken);
				}
				else
				{
					printf("    {MetaType_%s, \"%s\", (uint32_t)&((%s *)0)->%s},\n",
						MemberTypeToken.text.c_str(), token.text.c_str(),
						StructTypeToken.text.c_str(),
						token.text.c_str());
				}
			}
			break;
		case TOKEN_EQUALS:
			is_assigment = true;
			break;
		case TOKEN_SEMICOLON:
		case TOKEN_EOF:
			parsing = false;
			break;
		}
	}
}

// TODO: Allow inheritance
// template arguments (e.g. map, vector etc...)
// virtual functions

/* EXAMPLE OF META DATA GENERATED ?????
member_defintion_t MembersOf_entity[] =
{
	{MetaType_uuid, "id", (uint32_t) & ((entity*)0)->id},
	{MetaType_ENTITY_FLAGS, "flags", (uint32_t) & ((entity*)0)->flags},
	{MetaType_string, "tag", (uint32_t) & ((entity*)0)->tag},
	{MetaType_entity_manager, "manager", (uint32_t) & ((entity*)0)->manager},
	{MetaType_ivec3, "grid_pos", (uint32_t) & ((entity*)0)->grid_pos},
	{MetaType_ivec3, "previous_grid_pos", (uint32_t) & ((entity*)0)->previous_grid_pos},
	{MetaType_vec3, "visual_pos", (uint32_t) & ((entity*)0)->visual_pos},
	{MetaType_vec3, "vel", (uint32_t) & ((entity*)0)->vel},
};

member_defintion_t MembersOf_player_entity[] =
{
	{MetaType_uuid, "id", (uint32_t) & ((player_entity*)0)->id},
	{MetaType_ENTITY_FLAGS, "flags", (uint32_t) & ((player_entity*)0)->flags},
	{MetaType_string, "tag", (uint32_t) & ((player_entity*)0)->tag},
	{MetaType_entity_manager, "manager", (uint32_t) & ((player_entity*)0)->manager},
	{MetaType_ivec3, "grid_pos", (uint32_t) & ((player_entity*)0)->grid_pos},
	{MetaType_ivec3, "previous_grid_pos", (uint32_t) & ((player_entity*)0)->previous_grid_pos},
	{MetaType_vec3, "visual_pos", (uint32_t) & ((player_entity*)0)->visual_pos},
	{MetaType_vec3, "vel", (uint32_t) & ((player_entity*)0)->vel},

	{MetaType_model_entity, "model", (uint32_t) & ((player_entity*)0)->model},
	{MetaType_ivec2, "direction", (uint32_t) & ((player_entity*)0)->direction},
	{MetaType_float, "walk_speed", (uint32_t) & ((player_entity*)0)->walk_speed},
	{MetaType_float, "run_speed", (uint32_t) & ((player_entity*)0)->run_speed},
	{MetaType_float, "interp_speed", (uint32_t) & ((player_entity*)0)->interp_speed},
	{MetaType_vec3, "cam_original_pos", (uint32_t) & ((player_entity*)0)->cam_original_pos},
	{MetaType_float, "camera_speed", (uint32_t) & ((player_entity*)0)->camera_speed},
	{MetaType_vec3, "fp_look_rotation", (uint32_t) & ((player_entity*)0)->fp_look_rotation},
	{MetaType_vec3, "fp_look_direction", (uint32_t) & ((player_entity*)0)->fp_look_direction},
	{MetaType_bool, "is_first_person", (uint32_t) & ((player_entity*)0)->is_first_person},
};
*/

static void
ParseBaseStructAndClass(tokenizer_t* tokenizer)
{
}

static void
ParseStructAndClass(tokenizer_t* tokenizer)
{
	token_t NameToken = GetToken(tokenizer);

	token_t token = GetToken(tokenizer);
	if (token.type == TOKEN_COLON)
	{
		token = GetToken(tokenizer);
		if (TokenEquals(token, "public"))
		{
			token_t BaseNameToken = GetToken(tokenizer);

			ParseBaseStructAndClass(tokenizer);

			token = GetToken(tokenizer);
		}
	}

	if (token.type == TOKEN_OPEN_BRACE)
	{
		printf("member_defintion_t MembersOf_%s[] = \n", NameToken.text.c_str());
		printf("{\n");
		for (;;)
		{
			token_t MemberToken = GetToken(tokenizer);
			
			if (MemberToken.type == TOKEN_CLOSE_BRACE)
			{
				break;
			}
			else
			{
				if (MemberToken.text == "const")
				{
					// dont parse const variables
					MemberToken = GetToken(tokenizer);
				}

				if (MemberToken.text == "virtual")
				{
					MemberToken = GetToken(tokenizer);
				}

				if (PeakNextToken(tokenizer, TOKEN_NAMESPACE))
				{
					ConsumeToken(tokenizer);
					MemberToken = GetToken(tokenizer);
				}

				ParseMember(tokenizer, NameToken, MemberToken);
			}
		}

		printf("};\n");
	}
}

static void
ParseIntrospectable(tokenizer_t* tokenizer)
{
	if (RequireToken(tokenizer, TOKEN_OPEN_PAREN))
	{
		ParseIntrospectionParams(tokenizer);

		token_t TypeToken = GetToken(tokenizer);
		if (TokenEquals(TypeToken, "struct") ||
			TokenEquals(TypeToken, "class"))
		{
			ParseStructAndClass(tokenizer);
		}
		else
		{
			std::cout << "ERROR: Introspection is only supported for structs right now." << std::endl;
		}
	}
	else
	{
		std::cout << "ERROR: Missing parenthesis." << std::endl;
	}
}

int
main()
{
	std::string file_contents = read_entire_file(
		"D:\\jimbo\\Desktop\\GameProject\\GameProject\\src\\player.h");

	//std::cout << file_contents << std::endl;;

	tokenizer_t tokenizer = {};
	tokenizer.text = file_contents;

	bool parsing = true;
	while (parsing)
	{
		token_t token = GetToken(&tokenizer);
		switch (token.type)
		{
		case TOKEN_EOF:
			parsing = false;
			break;
		case TOKEN_UNKNOWN:
			break;
		case TOKEN_IDENTIFIER:
			if (TokenEquals(token, "introspect"))
			{
				ParseIntrospectable(&tokenizer);
			}
			break;
		default:
			//std::cout << (int)token.type << ": " << token.text << std::endl;
			break;
		}
	}
}