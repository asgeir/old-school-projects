#include "Lexer.h"
#include "Parser.h"
#include "TacBuilder.h"
#include "Token.h"

#include <boost/shared_ptr.hpp>

#include <iostream>

int main(int argc, char const *argv[])
{
    boost::shared_ptr<Lexer> lexer(new Lexer);
    boost::shared_ptr<Parser> parser(new Parser);
    boost::shared_ptr<TacBuilder> builder(new TacBuilder);

    ProgramPtr program = parser->parse(lexer);

    if (parser->errorCount() > 0) {
        std::cout << "Number of errors: " << parser->errorCount() << std::endl;
    } else {
        program->codegen(builder);

        std::cout << builder->output() << std::endl;
    }

	return 0;
}
