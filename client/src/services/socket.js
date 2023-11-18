import { SERVER_ADDR } from "../shared/constants/app";

export const webSocket = () => new WebSocket(SERVER_ADDR);