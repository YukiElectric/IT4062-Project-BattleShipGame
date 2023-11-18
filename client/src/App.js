import React, { useState, useEffect } from 'react';
import { webSocket } from './services/socket';

const App = () => {
    const [socket, setSocket] = useState(null);
    const [messages, setMessages] = useState([]);
    const [inputMessage, setInputMessage] = useState('');

    useEffect(() => {
        const newSocket = webSocket();
        setSocket(newSocket);

        newSocket.addEventListener('message', (event) => {
            setMessages((prevMessages) => [...prevMessages, event.data]);
        });

        return () => {
            newSocket.close();
        };
    }, []);

    const handleInputChange = (e) => {
        setInputMessage(e.target.value);
    };

    const handleSendMessage = () => {
        if (socket) {
            socket.send(inputMessage);
            setInputMessage('');
        }
    };

    return (
        <div>
            <h1>Test</h1>
            <div>
                {messages.map((message, index) => (
                    <div key={index}>{message}</div>
                ))}
            </div>
            <div>
                <input
                    type="text"
                    value={inputMessage}
                    onChange={handleInputChange}
                />
                <button onClick={handleSendMessage}>Send Message</button>
            </div>
        </div>
    );
};

export default App;
