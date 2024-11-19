# Hyper C Markup eXtension

The goal of this project is to implement a minimalistic web framework that uses C on the backend. This project will integrate with HTMX for the frontend and will use a custom markup language for the backend.

## Features

- [x] Custom markup language
- [x] HTMX integration
- [ ] Database integration

## Custom Markup Language

The custom markup language is a simple language that allows you to define routes and handlers in a simple way. It basically extends HTML with some custom tags.

### Example

```html
<?set id="title">
    My Awesome Web Page
</?set>

<?set id="user">
    <name>John Doe</name>
    <loggedIn>true</loggedIn>
</?set>

<html>
    <head>
        <title><?get id="title"></?get></title>
    </head>
    <body>
        <h1><?get id="title"></?get></h1>
        <p>Welcome, <?get id="user"><name></name></?get>!</p>
        <div hx-post="/update">
            <button>Click Me</button>
            <?hx-on>
                <?if>
                    <?get id="user"><loggedIn></loggedIn></?get>
                </?if>
                <?then>
                    <?return>
                        <p>Thank you for clicking the button, <?get id="user"><name></name></?get>!</p>
                    </?return>
                <?else>
                    <?return>
                        <p>Please log in to continue.</p>
                    </?return>
                <?/then>
            <?/hx-on>
        </div>
    </body>
</html>
```

**Evaluates to:**

```html
<html>
    <head>
        <title>My Awesome Web Page</title>
    </head>
    <body>
        <h1>My Awesome Web Page</h1>
        <p>Welcome, John Doe!</p>
        <div hx-post="/update">
            <button>Click Me</button>
        </div>
    </body>
</html>
```

**On the server side:**
```html
<?if>
    <?get id="user"><loggedIn></loggedIn></?get>
</?if>
<?then>
    <?return>
        <p>Thank you for clicking the button, <?get id="user"><name></name></?get>!</p>
    </?return>
<?else>
    <?return>
        <p>Please log in to continue.</p>
    </?return>
<?/then>
```

**