		VAR	i
		VAR	j
		VAR	__temp__1
		VAR	__temp__3
		VAR	__temp__8
		VAR	__temp__9
		VAR	__temp__10
		GOTO	example
example:		ASSIGN	1	i
		UMINUS	1	__temp__1
		ASSIGN	__temp__1	j
		GT	i	0	__temp__label__4
		ASSIGN	0	__temp__3
		GOTO	__temp__label__5
__temp__label__4:		ASSIGN	1	__temp__3
__temp__label__5:		NE	__temp__3	0	__block__label__2
		GOTO	__block__label__6

__block__label__2:		MULT	j	3	__temp__8
		SUB	1	__temp__8	__temp__9
		ASSIGN	__temp__9	i
		GOTO	__block__label__7

__block__label__6:		ADD	i	1	__temp__10
		ASSIGN	__temp__10	i
		GOTO	__block__label__7

__block__label__7:		APARAM	i
		CALL	writeln


