#ifndef SEVERITY_H
#define SEVERITY_H

enum class severity : size_t
{
	trace = 0,
	debug = 1,
	info = 2,
	warning = 3,
	error = 4,
	fatal = 5
};

#endif /* SEVERITY_H */
