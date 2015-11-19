		VAR	x
		VAR	y
		GOTO	test
add:		FPARAM	x
		FPARAM	y
		VAR	__temp__2
		GOTO	__block__label__1
__block__label__1:		ADD	x	y	__temp__2
		ASSIGN	__temp__2	add
		RETURN

mult:		FPARAM	x
		FPARAM	y
		VAR	__temp__4
		GOTO	__block__label__3
__block__label__3:		MULT	x	y	__temp__4
		ASSIGN	__temp__4	mult
		RETURN

toscreen:		FPARAM	x
		VAR	y
		GOTO	__block__label__5
__block__label__5:		ASSIGN	x	y
		APARAM	y
		CALL	writeln
		RETURN

test:		ASSIGN	3	x
		APARAM	x
		CALL	toscreen
		APARAM	x
		APARAM	2
		CALL	add
		ASSIGN	add	x
		APARAM	x
		CALL	toscreen
		APARAM	x
		APARAM	3
		CALL	mult
		ASSIGN	mult	x
		APARAM	x
		CALL	toscreen


