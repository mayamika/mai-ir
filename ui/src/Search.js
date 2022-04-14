import React, { useEffect, useState } from 'react';
import {
  useNavigate,
  useSearchParams,
  createSearchParams,
} from 'react-router-dom';

import { Stack, Container, Row } from 'react-bootstrap';

import SearchBar from './components/SearchBar';
import api from './api';

function Item(props) {
  const url = `https://vndb.org/${props.id}`;
  const title = (props.title) ? props.title : props.id;

  let description = props.description;
  if (description.length > 300) {
    description = description.substr(0, 300) + '...';
  }

  return (
    <Container>
      <Row>
        <a
          href={url}
          className="h5 link-primary text-decoration-none"
        >{title}</a>
      </Row>
      <Row>
        <p className="text-black-50">
          {description}
        </p>
      </Row>
    </Container>
  );
}

function Search() {
  const [searchParams] = useSearchParams();
  const navigate = useNavigate();

  const onSubmit = (query) => {
    const loc = {
      pathname: '/search',
      search: `?${createSearchParams({ query: query })}`,
    };
    navigate(loc);
  };

  const [items, setItems] = useState([]);
  useEffect(() => {
    const query = searchParams.get('query');
    if (!query) {
      return;
    }

    api.get(`/search?query=${query}`)
      .then((res) => {
        const data = res.data;
        if (!data.items) data.items = [];
        setItems(data.items);
      });
  }, [searchParams]);

  return (
    <Container className="my-4">
      <div className="sticky-top my-4">
        <Container>
          <SearchBar
            text={searchParams.get('query')}
            onSubmit={onSubmit}
          />
        </Container>
      </div >
      <Stack className="my-4" gap={2}>
        {items.map((item, i) => {
          return (
            <Item
              key={i}
              id={item.id}
              title={item.title}
              image={item.image}
              description={item.description}
            />
          );
        })}
      </Stack>
    </Container >
  );
}

export default Search;
