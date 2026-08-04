#include <csim.h>

extern csim_component_t altera_ParPort_component;


csim_component_t **altera_get_components() {
	static csim_component_t *components[] = {
		&altera_ParPort_component,
		NULL
	};
	return components;
}

