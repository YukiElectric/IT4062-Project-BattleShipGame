import { SERVER_ADDR } from "../shared/constant/app";

export const webSocket = () => new WebSocket(SERVER_ADDR);