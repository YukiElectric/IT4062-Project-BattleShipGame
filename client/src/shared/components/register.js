import React from "react";
import { Link, useNavigate } from "react-router-dom";
import { webSocket } from "../../services/socket";

const Register = () => {
    const [data, setData] = React.useState([]);
    const [socket, setSocket] = React.useState(null);
    const navigate = useNavigate();

    const onChangeValue = (e) => {
        const {name, value} = e.target;
        setData({...data, [name] : value});
    }

    const submitRegister = (e) => {
        e.preventDefault();
        const send = {request : "register", data};
        console.log(send);
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
            <h3>Sign Up</h3>

            <div className="mb-3">
                <label>User name</label>
                <input onChange={onChangeValue} name="user_name" required  type="text" className="form-control" placeholder="User name" value={data.user_name || ""}/>
            </div>
            <div className="mb-3">
                <label>Email address</label>
                <input
                    name="email"
                    onChange={onChangeValue}
                    required
                    type="email"
                    className="form-control"
                    placeholder="Enter email"
                    value={data.email || ""}
                />
            </div>
            <div className="mb-3">
                <label>Password</label>
                <input
                    name="password"
                    onChange={onChangeValue}
                    required
                    type="password"
                    className="form-control"
                    placeholder="Enter password"
                    value={data.password || ""}
                />
            </div>
            <div className="d-flex justify-content-center align-items-center">
                <button onClick={submitRegister} type="submit" className="btn btn-primary">
                    Sign Up
                </button>
            </div>
            <p className="forgot-password text-right">
                Already registered <Link to="/login">sign in?</Link>
            </p>
        </form>
    )
}

export default Register;