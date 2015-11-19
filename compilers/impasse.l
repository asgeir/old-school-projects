%option c++
%option yylineno
%option noyywrap
%option caseless
%option yyclass="Lexer"

%x COMMENT
%x ERROR

%{
#include "Lexer.h"
%}

LETTER      [a-zA-Z]
DIGIT       [0-9]
DIGITS      {DIGIT}+
FRACTION    \.{DIGITS}
EXPONENT    (E|e)(\+|-)?{DIGITS}
REAL        {DIGITS}({FRACTION}|{FRACTION}{EXPONENT}|{EXPONENT})

%%

%{
#include "Token.h"
%}

\n                                  { resetLine();       }
[ \t\r\f\v]                         { ignore();          }

\{                                  { BEGIN(COMMENT);    }
<COMMENT>\}                         { BEGIN(INITIAL);    }
<COMMENT>\n                         { resetLine();       }
<COMMENT>.                          { ignore();          }

:=                                  { return oper(TokenType::Assign);               }
not                                 { return oper(TokenType::Not);                  }
\.\.                                { return oper(TokenType::Range);                }
\+                                  { return addOp(OperatorType::Add);              }
-                                   { return addOp(OperatorType::Subtract);         }
or                                  { return addOp(OperatorType::Or);               }
and                                 { return mulOp(OperatorType::And);              }
\/                                  { return mulOp(OperatorType::Divide);           }
div                                 { return mulOp(OperatorType::IntegerDivide);    }
mod                                 { return mulOp(OperatorType::Modulus);          }
\*                                  { return mulOp(OperatorType::Multiply);         }
=                                   { return relOp(OperatorType::Equal);            }
\>                                  { return relOp(OperatorType::Greater);          }
\>=                                 { return relOp(OperatorType::GreaterOrEqual);   }
\<                                  { return relOp(OperatorType::Less);             }
\<=                                 { return relOp(OperatorType::LessOrEqual);      }
\<\>                                { return relOp(OperatorType::NotEqual);         }

\[                                  { return punctuation(TokenType::LBracket);      }
\]                                  { return punctuation(TokenType::RBracket);      }
:                                   { return punctuation(TokenType::Colon);         }
,                                   { return punctuation(TokenType::Comma);         }
\(                                  { return punctuation(TokenType::LParen);        }
\)                                  { return punctuation(TokenType::RParen);        }
\.                                  { return punctuation(TokenType::Period);        }
;                                   { return punctuation(TokenType::Semicolon);     }

array								{ return keyword(TokenType::Array);         }
begin                               { return keyword(TokenType::Begin);         }
do                                  { return keyword(TokenType::Do);            }
else                                { return keyword(TokenType::Else);          }
end                                 { return keyword(TokenType::End);           }
function                            { return keyword(TokenType::Function);      }
if                                  { return keyword(TokenType::If);            }
integer                             { return keyword(TokenType::Integer);       }
of                                  { return keyword(TokenType::Of);            }
procedure                           { return keyword(TokenType::Procedure);     }
program                             { return keyword(TokenType::Program);       }
real                                { return keyword(TokenType::Real);          }
then                                { return keyword(TokenType::Then);          }
var                                 { return keyword(TokenType::Var);           }
while                               { return keyword(TokenType::While);         }

{LETTER}({LETTER}|{DIGIT})*         { return identifier();                      }
{DIGITS}                            { return number(NumberType::Integer);       }
{REAL}                              { return number(NumberType::Real);          }

.                                   { return error();   }

<<EOF>>                             { return eof();     }

%%
