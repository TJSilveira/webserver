NAME	= webserv

CXX		= c++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -fsanitize=address

SRC_DIR			= ./src/
OBJS_DIR		= ./objs/

SRC_FILES		=	main.cpp \
					Lexer.cpp \
					Parser.cpp \
					Server.cpp \
					VirtualServer.cpp \
					Location.cpp \
					utils.cpp


OBJS_FILES			= $(SRC_FILES:.cpp=.o)
OBJS				= $(addprefix $(OBJS_DIR), $(OBJS_FILES))


${NAME}: ${OBJS}
	${CXX} ${CXXFLAGS} -o $@ ${OBJS}

$(OBJS_DIR)%.o: $(SRC_DIR)%.cpp | $(OBJS_DIR)
	${CXX} ${CXXFLAGS} -c $< -o $@

# Creating Objects dir
$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

all: ${NAME}

clean:
	@echo ""
	@echo "$(RED)Did someone call - $(ITALIC)pause$(RESET)$(RED) - the Clean-up crew?!$(RESET)"
	@ rm -rf $(OBJS_DIR)

fclean: clean
	rm -f ${NAME}

re: fclean ${NAME}
