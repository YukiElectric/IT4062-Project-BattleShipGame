import React from "react";
import { webSocket } from "../../services/socket";

const Login = () => {
    const [input, setInput] = React.useState([]);
    const [socket, setSocket] = React.useState(null);

    const onchangeValue = (e) => {
        const {name, value} = e.target;
        setInput({...input, [name] : value});
    }

    const submitLogin = (e) => {
        const data = {...input};
        const send = {request : "login", data};
        console.log(send);
        e.preventDefault();
        if(socket) {
            socket.send(JSON.stringify(send));
        }
    }

    React.useEffect(()=>{
        const newSocket = webSocket();
        setSocket(newSocket);
        newSocket.addEventListener("message", ({data})=>{
           if(data) console.log(JSON.parse(data));
        });
        return () => {
            newSocket.close();
        }
    },[])
    
    return (
        <form>
            <h3>Sign In</h3>

            <div className="mb-3">
                <label>User name</label>
                <input
                    onChange={onchangeValue}
                    name="user_name"
                    required
                    type="text"
                    className="form-control"
                    placeholder="Enter user name"
                    value={input.user_name || ""}
                />
            </div>
            <div className="mb-3">
                <label>Password</label>
                <input
                    onChange={onchangeValue}
                    name="password"
                    required
                    type="password"
                    className="form-control"
                    placeholder="Enter password"
                    value={input.password || ""}
                />
            </div>
            <div className="mb-3">
                <div className="custom-control custom-checkbox">
                    <input
                        type="checkbox"
                        className="custom-control-input"
                        id="customCheck1"
                    />
                    <label className="custom-control-label" htmlFor="customCheck1">Remember me</label>
                </div>
            </div>
            <div className="d-flex justify-content-center align-items-center">
                <button onClick={submitLogin} type="submit" className="btn btn-primary">Submit</button>
            </div>
            <p className="forgot-password text-right">
                Forgot <a href="#">password?</a>
            </p>
        </form>
    )
}

export default Login;