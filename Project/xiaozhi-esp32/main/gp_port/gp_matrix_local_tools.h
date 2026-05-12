#ifndef GP_MATRIX_LOCAL_TOOLS_H_
#define GP_MATRIX_LOCAL_TOOLS_H_

class Display;
class McpServer;
class GpLedMatrixEsp32;

/* Register matrix drawing tools that can run locally on AI side without host scripts. */
void RegisterGpMatrixLocalMcpTools(McpServer& server,
                                   GpLedMatrixEsp32* matrix_led,
                                   Display* display);

#endif
