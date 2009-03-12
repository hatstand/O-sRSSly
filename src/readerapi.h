#ifndef READERAPI_H
#define READERAPI_H

#include "apiaction.h"
#include "atomfeed.h"
#include "subscriptionlist.h"

#include <list>
#include <string>

#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QTime>
#include <QTimer>
#include <QXmlStreamReader>

#include <boost/mpl/list.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/utility.hpp>

class Database;
namespace sc = boost::statechart;
namespace mpl = boost::mpl;


class ReaderApi : public QObject, boost::noncopyable {
Q_OBJECT
public:
	ReaderApi(Database* db, QObject* parent = 0);
	virtual ~ReaderApi();

	bool isLoggedIn();
	void login(const QString& username, const QString& password);
	void getSubscriptionList();
	void getSubscription(const Subscription& s, const QString& continuation = "");
	void getFresh();
	void getUnread();
	void getCategory(const QString& category, const QString& continuation = "");
	void getFriends();
	void setRead(const AtomEntry& e);
	void setStarred(const AtomEntry& e, bool starred);
	void setShared(const AtomEntry& e, bool shared);
	void addCategory(const Subscription& s, const QString& category);
	void removeCategory(const Subscription& s, const QString& category);

	void search(const QString& query);
	void subscribe(const QString& feed);

private:
	void getToken();
	void editCategory(const Subscription& s, const QString& category, bool add);
	void setState(const AtomEntry& e, const char* state, bool set);
	void getSubscription(const QString& id, const QString& continuation = "");
	void getSubscription(const QString& id, int count, const QString& timestamp);
	void getSubscription(const QUrl& url);
	QUrl encodeFeedId(const QString& id);

	QMap<QString, QPair<int, QString> > parseUnreadCounts(QIODevice* device);
	void parseFeedUnreadCount(QXmlStreamReader& s, QMap<QString, QPair<int, QString> >* unread_counts);
	
	void watchReply(QNetworkReply* reply);
	void updateProgress();

	// Checks whether this request has been submitted recently.
	bool checkThrottle(const QUrl& url);

        // Parses the intermediate search results into individual ids.
	QStringList parseIntermediateSearch(QIODevice* data);

private slots:
	void loginComplete();
	void loginFailed(QNetworkReply::NetworkError);
	void getSubscriptionListComplete();
	void getSubscriptionComplete();
	void getFreshComplete();
	void getUnreadComplete();
	void getCategoryComplete();
	void getFriendsComplete();
	void getTokenComplete();
	void networkError(QNetworkReply::NetworkError code);
	void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
	void processActionQueue();
	void actionFailed();
        // First part of search - getting ids.
	void searchPart();
        // Search complete - full detail.
	void searchFinished();
	
	void replyDownloadProgress(qint64, qint64);
	void replyFinished();

	// Call to clean out the network_throttle_ of old entries 
	void clearThrottle();

	void subscribeFinished();

signals:
	void loggedIn(bool success);
	void subscriptionListArrived(const SubscriptionList&);
	void subscriptionArrived(const AtomFeed&);
	void categoryArrived(const AtomFeed&);
	void freshArrived(const AtomFeed&);
	void friendsArrived(const AtomFeed&);
	// Emitted when an auth token has been received.
	void tokenReady();
	
	void progressChanged(int progress, int total);

private:
	QMap<QNetworkReply*, int> reply_progress_;
	
	QNetworkAccessManager* network_;

	// Needed for editing things on Reader
	QString auth_;
	// Used in the cookie to show who we are
	QString sid_;

	// Temporary authentication token.
	QString token_;
	QQueue<ApiAction*> queued_actions_;

	// Reading list continuation.
	QString continuation_;
        // Shared items continuation.
	QString friends_continuation_;

	QMap<QUrl, QTime> network_throttle_;
	QTimer throttle_clear_;

	Database* db_;

	// States:
	struct Active;
	
	struct NotLoggedIn;
	struct LoggingIn;
	struct LoggedIn;

	struct NoToken;
	struct GettingToken;
	struct GotToken;

	// Events:
	struct StartLogin : sc::event<StartLogin> {};
	struct LoginSuccess : sc::event<LoginSuccess> {};
	struct LoginFail : sc::event<LoginFail> {};

	struct GetToken : sc::event<GetToken> {};
	struct GetTokenSuccess : sc::event<GetTokenSuccess> {};
	struct GetTokenFail : sc::event<GetTokenFail> {};
	struct TokenExpired : sc::event<TokenExpired> {};

	// State machine:
	struct State : sc::state_machine<State, Active> {
		State(ReaderApi* parent) : parent_(parent) {}

		ReaderApi* parent_;
	};

	struct Active : sc::simple_state<Active, State, mpl::list<NotLoggedIn, NoToken> > {};

	struct NotLoggedIn : sc::state<NotLoggedIn, Active::orthogonal<0> > {
		NotLoggedIn(my_context ctx) : sc::state<NotLoggedIn, Active::orthogonal<0> >(ctx) {
			qDebug() << __PRETTY_FUNCTION__;
			emit(outermost_context().parent_->loggedIn(false));
		}
		~NotLoggedIn() { qDebug() << __PRETTY_FUNCTION__; }

		typedef sc::transition<StartLogin, LoggingIn> reactions;
	};

	struct LoggingIn : sc::simple_state<LoggingIn, Active::orthogonal<0> > {
		LoggingIn() { qDebug() << __PRETTY_FUNCTION__; }
		~LoggingIn() { qDebug() << __PRETTY_FUNCTION__; }

		typedef mpl::list<
			sc::transition<LoginSuccess, LoggedIn>,
			sc::transition<LoginFail, NotLoggedIn>
		> reactions;
	};

	struct LoggedIn : sc::state<LoggedIn, Active::orthogonal<0> > {
		LoggedIn(my_context ctx) : sc::state<LoggedIn, Active::orthogonal<0> >(ctx) {
			qDebug() << __PRETTY_FUNCTION__;
			emit(outermost_context().parent_->loggedIn(true));
		}
		~LoggedIn() { qDebug() << __PRETTY_FUNCTION__; }
	};

	struct NoToken : sc::simple_state<NoToken, Active::orthogonal<1> > {
		NoToken() { qDebug() << __PRETTY_FUNCTION__; }

		typedef sc::transition<GetToken, GettingToken> reactions;
	};

	struct GettingToken : sc::simple_state<GettingToken, Active::orthogonal<1> > {
		GettingToken() { qDebug() << __PRETTY_FUNCTION__; }

		typedef mpl::list<
			sc::transition<GetTokenSuccess, GotToken>,
			sc::transition<GetTokenFail, NoToken>
		> reactions;
	};

	struct GotToken : sc::simple_state<GotToken, Active::orthogonal<1> > {
		GotToken() { qDebug() << __PRETTY_FUNCTION__; }

		typedef sc::transition<TokenExpired, NoToken> reactions;
	};

	State state_;

	template <typename T>
	bool isCurrentState() {
		return state_.state_downcast<const T*>();
	}

public:
	static const char* kApplicationSource;
	static const char* kServiceName;
	static const char* kAccountType;

	static const QUrl kLoginUrl;

	static const QUrl kSubscriptionUrl;
	static const QUrl kTagsUrl;
	static const QUrl kUnreadUrl;
	static const QUrl kPrefsUrl;

	static const QUrl kTokenUrl;
	
	static const QUrl kEditTagUrl;
	static const char* kReadTag;
	static const char* kStarredTag;
	static const char* kFreshTag;
	static const char* kFriendsTag;
	static const char* kSharedTag;
	static const QUrl kEditSubscriptionUrl;

	static const QUrl kAtomUrl;

	static const char* kReadingList;

	static const QUrl kSearchUrl;
	static const QUrl kIdConvertUrl;

	static const QUrl kSubscribeUrl;
	static const QUrl kCommentUrl;
	static const QUrl kSetPreferenceUrl;
};

#endif
