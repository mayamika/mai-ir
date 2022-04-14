import React from 'react';
import { BrowserRouter, Routes, Route } from 'react-router-dom';

import { Container } from 'react-bootstrap';
import Search from './Search';

function App() {
  return (
    <div>
      <Container>
        <BrowserRouter>
          <Routes>
            <Route path="/search" element={<Search />} />
            <Route path="/" element={<Search />} />
          </Routes>
        </BrowserRouter>
      </Container>
    </div >
  );
}

export default App;
