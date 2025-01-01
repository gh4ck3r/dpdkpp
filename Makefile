VM := vm-dpdk
.DEFAULT_GOAL := all
%::
	@ssh -tt ${VM} ${MAKE} --no-print-directory -C ${PWD}/build ${MAKECMDGOALS}
