target:
	@mkdir -p dist
	@gcc -w src/server.c -o dist/db
clean:
	@rm dist/*

