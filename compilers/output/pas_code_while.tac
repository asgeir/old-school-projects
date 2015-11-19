		VAR	i
		VAR	j
		VAR	__temp__3
		VAR	__temp__4
		VAR	__temp__5
		GOTO	example
example:		ASSIGN	0	i
		ASSIGN	1	j
		GOTO	__block__label__1

__block__label__1:		LT	i	10	__temp__label__6
		ASSIGN	0	__temp__5
		GOTO	__temp__label__7
__temp__label__6:		ASSIGN	1	__temp__5
__temp__label__7:		NE	__temp__5	0	__block__label__2
		GOTO	__block__label__8

__block__label__2:		ADD	i	j	__temp__3
		ASSIGN	__temp__3	j
		ADD	i	1	__temp__4
		ASSIGN	__temp__4	i
		GOTO	__block__label__1

__block__label__8:		APARAM	i
		CALL	writeln
		APARAM	j
		CALL	writeln


