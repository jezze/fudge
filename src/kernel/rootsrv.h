void rootsrv_registernode(struct service *service, char *name, struct resource_operands *operands);
void rootsrv_registerdirectory(struct service *service, char *name, unsigned int type);
void rootsrv_unregisternode(struct service *service);
void rootsrv_unregisterdirectory(struct service *service);
void rootsrv_setup(void);
