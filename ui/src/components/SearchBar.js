import React, { useState } from 'react';

import { Form, FormControl } from 'react-bootstrap';

function SearchBar(props) {
  const { text } = props;
  const [query, setQuery] = useState(text);

  const onTextInput = (e) => {
    e.preventDefault();
    setQuery(e.target.value);
  };
  const onFormSubmit = (e) => {
    e.preventDefault();
    if (props.onSubmit) {
      props.onSubmit(query);
    }
  };

  return (
    <Form className="d-flex" onSubmit={onFormSubmit}>
      <FormControl
        type="search"
        placeholder="Enter query"
        className="me-2"
        aria-label="Search"
        onChange={onTextInput}
      />
    </Form>
  );
}

export default SearchBar;
