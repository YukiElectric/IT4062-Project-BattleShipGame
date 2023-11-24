import React from 'react'
import '../node_modules/bootstrap/dist/css/bootstrap.min.css'
import './App.css'
import Login from './shared/components/login'
import Register from './shared/components/register'
import { Link, Route, Routes, BrowserRouter } from 'react-router-dom'

const App = () => {
    return (
        <BrowserRouter>
            <div className="App">
                <nav className="navbar navbar-expand-lg navbar-light fixed-top">
                    <div className="container">
                        <Link className="navbar-brand" to={'/login'}>KGRF</Link>
                        <div className="collapse navbar-collapse" id="navbarTogglerDemo02">
                            <ul className="navbar-nav ml-auto">
                                <li className="nav-item"><Link className="nav-link" to={'/login'}>Login</Link></li>
                                <li className="nav-item"><Link className="nav-link" to={'/register'}>Register</Link></li>
                            </ul>
                        </div>
                    </div>
                </nav>
                <div className="auth-wrapper">
                    <div className="auth-inner">
                        <Routes>
                            <Route path="/" element={<Login />} />
                            <Route path="/login" element={<Login />} />
                            <Route path="/register" element={<Register />} />
                        </Routes>
                    </div>
                </div>
            </div>
        </BrowserRouter>
    )
}

export default App;